#include "meep-ctl.hpp"

using namespace meep;

/**************************************************************************/

/* The following are hook functions called from main() when
   starting the program and just before exiting.  */

static initialize *meep_init = 0;

void ctl_start_hook(int *argc, char ***argv)
{
  meep_init = new initialize(*argc, *argv);
}

void ctl_stop_hook(void)
{
  delete meep_init;
}

extern "C" void SWIG_init();

void ctl_export_hook(void)
{
  SWIG_init();
}

/**************************************************************************/

ctlio::cvector3_list do_harminv(ctlio::cnumber_list vals, double dt, 
				double fmin, double fmax, int maxbands)
{
  complex<double> *amp = new complex<double>[maxbands];
  double *freq_re = new double[maxbands];
  double *freq_im = new double[maxbands];
  double *freq_err = new double[maxbands];
  maxbands = do_harminv(reinterpret_cast<complex<double>*>(vals.items),
			vals.num_items, dt, fmin, fmax, maxbands,
			amp, freq_re, freq_im, freq_err);
  ctlio::cvector3_list res;
  res.num_items = maxbands;
  res.items = new cvector3[maxbands];
  for (int i = 0; i < maxbands; ++i) {
    res.items[i].x.re = freq_re[i];
    res.items[i].x.im = freq_im[i];
    res.items[i].y.re = real(amp[i]);
    res.items[i].y.im = imag(amp[i]);
    res.items[i].z.re = freq_err[i];
    res.items[i].z.im = 0;
  }
  delete[] freq_err;
  delete[] freq_im;
  delete[] freq_re;
  delete[] amp;
  return res;
}

/**************************************************************************/

/* This is a wrapper function to fool SWIG...since our list constructor
   takes ownership of the next pointer, we have to make sure that SWIG
   does not garbage-collect geometric_volume_list objects.  We do
   this by wrapping a "helper" function around the constructor which
   does not have the %newobject SWIG attribute.   Note that we then
   need to deallocate the list explicitly in Scheme. */
geometric_volume_list
  *make_geometric_volume_list(const geometric_volume &gv,
			      int c, complex<double> weight,
			      geometric_volume_list *next)
{
  return new geometric_volume_list(gv, c, weight, next);
}

/***************************************************************************/

ctlio::number_list dft_flux_flux(dft_flux *f)
{
  ctlio::number_list res;
  res.num_items = f->Nfreq;
  res.items = f->flux();
  return res;
}

/***************************************************************************/

ctlio::cnumber_list make_casimir_g(double T, double dt, double sigma, 
				   complex<double> (*eps_func)(complex<double> omega),
				   double Tfft)
{
  ctlio::cnumber_list res;
  res.num_items = ceil(T / dt);
  res.items = new cnumber[res.num_items];
  complex<double> *g = meep::make_casimir_g(T, dt, sigma, eps_func, Tfft);
  for (int i = 0; i < res.num_items; ++i) {
    res.items[i].re = real(g[i]);
    res.items[i].im = imag(g[i]);
  }
  delete[] g;
  return res;
}
