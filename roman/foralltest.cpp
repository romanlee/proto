// modified from tests/ForallTests.cpp
// #include <gtest/gtest.h>
#include "Proto.H"
#include <filesystem>

using namespace Proto;

#define NUMCOMPS DIM+2
#define GET_TIMINGS 1

PROTO_KERNEL_START
void f_threshold_temp( Var<short>& a_tags,
             Var<double, NUMCOMPS>& a_U)
{
  double thresh = 1.001;
  if (a_U(0) > thresh) {a_tags(0) = 1;}
  else {a_tags(0) = 0;};
};
PROTO_KERNEL_END(f_threshold_temp, f_threshold);

PROTO_KERNEL_START
void consToPrim_temp(Var<double,DIM+2>& W, 
                     const Var<double, DIM+2>& U,
                     double gamma, int N_int)
{
  // Heavy kernel loop
  for (int j=0; j<N_int; j++){
    double rho = U(0);
    double v, v2=0.;
    W(0) = rho;
    
    for (int i = 1; i <= DIM; i++)
      {
        v = U(i) / rho;
        
        W(i) = v;
        v2 += v*v;
      }
    
    W(DIM+1) = (U(DIM+1)-.5*rho*v2) * (gamma-1.);
  }
}
PROTO_KERNEL_END(consToPrim_temp, consToPrim)

int main(){

  // TODO make these hyper params
  const int nx = 16;
  const int N_ext = 10; // Set >1 for "light kernel" test, else  set =1
  const int N_int = 1; // Set >1 for "heavy kernel" test, else set =1

#ifdef GET_TIMINGS
  std::filesystem::path currentPath = std::filesystem::current_path();
  std::filesystem::path fullPath = currentPath / "TIMINGS.txt"; 
  PR_TIMER_SETFILE(fullPath.string());
#endif 

  // std::cout << "Default memory type: " << parseMemType(MEMTYPE_DEFAULT) << std::endl;

  // Set up the input array
  Box srcBox = Box::Cube(nx);

  BoxData<double,DIM+2> U(srcBox,1);
  BoxData<double,DIM+2> W(srcBox,1);
  BoxData<double,DIM+2> V(srcBox,1);
  BoxData<short> X(srcBox);
  BoxData<double,DIM+2> Y(srcBox,1);

  // BoxData<double,DIM+2> a_U(srcBox,1);
  // BoxData<double,DIM+2> U(srcBox,1);
  // BoxData<double,DIM+2> W(srcBox,1);
  // BoxData<double,DIM+2> W_bar(srcBox,1);
  // BoxData<double,DIM+2> W_ave(srcBox,1);

  // // from tests/Tutorial.cpp
  // Stencil<double> S = 0.5*Shift::X(-1) + 0.5*Shift::X(+1); // This means: S[D_{i}] = 0.5*D_{i-1} + 0.5*D_{i+1}

  const double gamma = 1.4;  

  // Light kernel loop
  {
    PR_TIME("forall_zone");

    for (int i=0; i<N_ext; i++){
      // forall with consToPrim
      // forallInPlace<double,DIM+2,MEMTYPE_DEFAULT,1,1>(consToPrim,W,U,gamma,N_int);
      forallInPlace(consToPrim,W,U,gamma,N_int);

      // // Interleaving forall with Stencil
      // forallInPlace(consToPrim,W,U,gamma,N_int);
      // V |= S(W);

      // // forall with f_threshold
      // forallInPlace(f_threshold,X,U);

      // // interleaving two different foralls
      // forallInPlace(consToPrim,W,U,gamma,N_int);
      // forallInPlace(f_threshold,X,U);

      // // interleaving the same forall on different datas
      // forallInPlace(consToPrim,W,U,gamma,N_int);
      // forallInPlace(consToPrim,V,Y,gamma,N_int);

      // // use pattern from BoxOp_Euler.H (except that I do all the operations in place)
      // forallInPlace(consToPrim, W_bar, a_U, gamma, N_int);
      // // U = Operator::deconvolve(a_U);
      // Operator::deconvolve(U, a_U);
      // forallInPlace(consToPrim, W, U, gamma, N_int);
      // // W_ave = Operator::_convolve(W, W_bar);
      // Operator::_convolve(W_ave, W, W_bar);
    }

    // ensures that the timer only stops once the last kernel is done
    cudaDeviceSynchronize();

  }

#ifdef GET_TIMINGS
  // Screws up Nsight systems (except when...the PR_TIMER_SETFILE is in the same dir as the .nsys-rep???)
  PR_TIMER_REPORT();
#endif

  // Check the reuslts (from ForallTests.cpp)
  // EXPECT_EQ(U.box(),W.box());
  // BoxData<double,DIM+2,HOST> U_host(srcBox), W_host(srcBox);
  // U.copyTo(U_host);
  // W.copyTo(W_host);
  // consToPrimCheck(U_host,W_host,gamma,srcBox);
  // Box destBox = Box::Cube(3);
  // BoxData<double,DIM+2> W2 = forall<double,DIM+2>(consToPrim,destBox,U,gamma);
  // EXPECT_EQ(W2.box(),destBox);
  // BoxData<double,DIM+2,HOST> W2_host(destBox);
  // W2.copyTo(W2_host);
  // consToPrimCheck(U_host,W2_host,gamma,destBox);

  return 0;
}