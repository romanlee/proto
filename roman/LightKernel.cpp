// modified from tests/ForallTests.cpp
#include <gtest/gtest.h>
#include "Proto.H"

#define NUMCOMPS DIM+2

using namespace Proto;

PROTO_KERNEL_START
void consToPrim_temp(Var<double,DIM+2>& W, 
                     const Var<double, DIM+2>& U,
                     double gamma)
{
  // printf("hello\n");

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
PROTO_KERNEL_END(consToPrim_temp, consToPrim)

int main(){

  std::cout << "Default memory type: " << parseMemType(MEMTYPE_DEFAULT) << std::endl;

  // PR_TIME("main");

  // Hyper params
  const int nx = 2;
  const int N = 10;

  cout << "DIM" << DIM << "\n";

  // Set up the input array
  Box srcBox = Box::Cube(nx);
  BoxData<double,DIM+2> U(srcBox,1);

  const double gamma = 1.4;  

  // Call light kernel N times
  for (int i=0; i<N; i++){
    cout << "i: " << i << "\n";
    BoxData<double,DIM+2> W = forall<double,DIM+2>(consToPrim,U,gamma);
  }
  cudaDeviceSynchronize();

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