// modified from tests/ForallTests.cpp
// #include <gtest/gtest.h>
#include "Proto.H"

using namespace Proto;

#define NUMCOMPS DIM+2

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

  PR_TIMER_SETFILE("roman/LightKernel/TIMINGS.txt")

  std::cout << "Default memory type: " << parseMemType(MEMTYPE_DEFAULT) << std::endl;

  // TODO make these hyper params
  const int nx = 512;
  const int N_ext = 10; // Set >1 for "light kernel" test, else  set =1
  const int N_int = 1; // Set > 1 for "heavy kernel" test, else set =1

  // Set up the input array
  Box srcBox = Box::Cube(nx);
  BoxData<double,DIM+2> U(srcBox,1);
  BoxData<double,DIM+2> W(srcBox,1);
  // BoxData<double,DIM+2> W;

  Stencil<double> S = 0.5*Shift::X(-1) + 0.5*Shift::X(+1); // This means: S[D_{i}] = 0.5*D_{i-1} + 0.5*D_{i+1}

  const double gamma = 1.4;  

  // Light kernel loop
  {
    PR_TIME("forall_zone");

    for (int i=0; i<N_ext; i++){
      // cout << "i: " << i << "\n";

      // auto W = forall<double,DIM+2>(consToPrim,U,gamma,N_int);
      // forallInPlace<double,DIM+2,MEMTYPE_DEFAULT,1,1>(consToPrim,W,U,gamma,N_int);
      forallInPlace(consToPrim,W,U,gamma,N_int);

      // try interleaving a stencil
      X += S(W);

      // Not sure this is necessary. This kernel seems to happen in serial no matter what
      // cudaDeviceSynchronize(); 
    }

  }

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

  // // Screws up Nsight systems except when...the PR_TIMER_SETFILE is in the same dir as
  // // the .nsys-rep???
  // PR_TIMER_REPORT();

  return 0;
}