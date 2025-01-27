  
#include "Preprocessor"

  #ifdef __cplusplus
   extern "C"
     {
   #endif

  void ComputeDifferences_C
      ( double *V, int *lV, int *uV, int *iaS, int iD,
        double *dV_Left, double *dV_Right, int *nSizes );
  void ComputeReconstruction_C
      ( double *V, int *dV_Left, int *dV_Right, double Theta,
        double *V_Inner, double *V_Outer, int nValues);
  void ComputeFluxes_C
      ( double *AP_I, double *AP_O, double *AM_I, double *AM_O,
        double *RF_I, double *RF_O, double *U_I, double *U_O,
        int *lV, int *uV, int *iaS, int iD, double *F_I, double *F_0,
        int *nSizes );
  void ComputeUpdate_C
      ( double *dU, double *F_I, double *F_O,
        double V, double A, double dT, int nValues );
  void AddUpdate_C ( double *O, double *U, double *C, int nValues );
  void CombineUpdates_C ( double *C, double *O, double *U, int nValues );

  #ifdef __cplusplus
    }
  #endif
 
void ComputeDifferences_C
    ( double *V, int *lV, int *uV, int *iaS, int iD,
        double *dV_Left, double *dV_Right, int *nSizes ) { 

    int iV, jV, kV, iaS, iaVS, lV, uV;
    int dV_iD, VS_iD;
      
    lV = 1
    where ( shape ( V ) > 1 )
      lV = oV + 1
    end where
    lV ( iD ) = oV

    uV = 1
    where ( shape ( V ) > 1 )
      uV = shape ( V ) - oV
    end where
    uV ( iD ) = size ( V, dim = iD ) - 1
    
!    dV_Left  = V - cshift ( V, shift = -1, dim = iD )    

    iaS = 0
    iaS ( iD ) = -1
    
    !$OMP  OMP_TARGET_DIRECTIVE parallel do simd collapse ( 3 ) &
    !$OMP& schedule ( OMP_SCHEDULE_TARGET ) private ( iaVS )
    do kV = lV ( 3 ), uV ( 3 ) 
      do jV = lV ( 2 ), uV ( 2 )
        do iV = lV ( 1 ), uV ( 1 )
        
          !call Show ( [ iV, jV, kV ], '<<< iV, jV, kV: 1' )

          iaVS = [ iV, jV, kV ] + iaS

          dV_Left ( iV, jV, kV )  &
            =  V ( iV, jV, kV )  -  V ( iaVS ( 1 ), iaVS ( 2 ), iaVS ( 3 ) )

        end do !-- iV
      end do !-- jV
    end do !-- kV
    !$OMP end OMP_TARGET_DIRECTIVE parallel do simd
    
!    dV_Right = cshift ( V, shift = 1, dim = iD ) - V

    iaS = 0
    iaS ( iD ) = +1
    
    !$OMP  OMP_TARGET_DIRECTIVE parallel do simd collapse ( 3 ) &
    !$OMP& schedule ( OMP_SCHEDULE_TARGET ) private ( iaVS )
    do kV = lV ( 3 ), uV ( 3 ) 
      do jV = lV ( 2 ), uV ( 2 )
        do iV = lV ( 1 ), uV ( 1 )
          
          !call Show ( [ iV, jV, kV ], '<<< iV, jV, kV: 2' )

          iaVS = [ iV, jV, kV ] + iaS

          dV_Right ( iV, jV, kV )  &
            =  V ( iaVS ( 1 ), iaVS ( 2 ), iaVS ( 3 ) ) - V ( iV, jV, kV )

        end do !-- iV
      end do !-- jV
    end do !-- kV
    !$OMP end OMP_TARGET_DIRECTIVE parallel do simd
}


  module procedure ComputeReconstructionKernel

    !real ( KDR ), dimension ( size ( V ) ) :: &
    !  dV
    integer ( KDI ) :: &
      iV
    real ( KDR ) :: &
      dV
      
    !where ( dV_Left > 0.0_KDR .and. dV_Right > 0.0_KDR )
    !  dV = min ( Theta * dV_Left, Theta * dV_Right, &
    !               0.5_KDR * ( dV_Left + dV_Right ) )
    !elsewhere ( dV_Left < 0.0_KDR .and. dV_Right < 0.0_KDR )
    !  dV = max ( Theta * dV_Left, Theta * dV_Right, &
    !               0.5_KDR * ( dV_Left + dV_Right ) )      
    !elsewhere
    !  dV = 0.0_KDR
    !endwhere

    !V_Inner = V - 0.5_KDR * dV
    !V_Outer = V + 0.5_KDR * dV
    
    !$OMP  OMP_TARGET_DIRECTIVE parallel do simd &
    !$OMP& schedule ( OMP_SCHEDULE_TARGET ) private ( dV )
    do iV = 1, size ( V )
      dV = ( sign ( 0.5_KDR, dV_Left ( iV ) ) &
             + sign ( 0.5_KDR, dV_Right ( iV ) ) ) &
             * min ( abs ( Theta * dV_Left ( iV ) ), &
                     abs ( Theta * dV_Right ( iV ) ), &
                     abs ( 0.5_KDR * ( dV_Left ( iV ) + dV_Right ( iV ) ) ) )
      V_Inner ( iV ) = V ( iV ) - 0.5_KDR * dV
      V_Outer ( iV ) = V ( iV ) + 0.5_KDR * dV
    end do
    !$OMP end OMP_TARGET_DIRECTIVE parallel do simd
    
  end procedure ComputeReconstructionKernel


  module procedure ComputeFluxesKernel

    integer ( KDI ) :: &
      iV, jV, kV
    integer ( KDI ), dimension ( 3 ) :: &
      iaS, &
      iaVS, &   
      lV, uV
      
    lV = 1
    where ( shape ( F_I ) > 1 )
      lV = oV + 1
    end where
    lV ( iD ) = oV

    uV = 1
    where ( shape ( F_I ) > 1 )
      uV = shape ( F_I ) - oV
    end where
    uV ( iD ) = size ( F_I, dim = iD ) - 1
    
    !where ( AP_I + AM_I > 0.0_KDR )
    !  F_I = ( AP_I * cshift ( RF_O, shift = -1, dim = iD )  +  AM_I * RF_I &
    !          - AP_I * AM_I * ( U_I - cshift ( U_O, shift = -1, dim = iD ) ) ) &
    !        / ( AP_I + AM_I )
    !elsewhere
    !  F_I = 0.0_KDR
    !end where
    
    iaS = 0
    iaS ( iD ) = -1
    
    !$OMP  OMP_TARGET_DIRECTIVE parallel do simd collapse ( 3 ) &
    !$OMP& schedule ( OMP_SCHEDULE_TARGET ) private ( iaVS )
    do kV = lV ( 3 ), uV ( 3 ) 
      do jV = lV ( 2 ), uV ( 2 )
        do iV = lV ( 1 ), uV ( 1 )
            
          iaVS = [ iV, jV, kV ] + iaS
          
          if ( AP_I ( iV, jV, kV ) + AM_I ( iV, jV, kV ) > 0.0_KDR ) then
            F_I ( iV, jV, kV ) &
              = ( AP_I ( iV, jV, kV ) &
                    * RF_O ( iaVS ( 1 ), iaVS ( 2 ), iaVS ( 3 ) ) &
                  + AM_I ( iV, jV, kV ) * RF_I ( iV, jV, kV ) &
                  - AP_I ( iV, jV, kV ) * AM_I ( iV, jV, kV ) &
                    * ( U_I ( iV, jV, kV ) &
                        -  U_O ( iaVS ( 1 ), iaVS ( 2 ), iaVS ( 3 ) ) ) ) &
                / ( AP_I ( iV, jV, kV ) + AM_I ( iV, jV, kV ) )
          else
            F_I ( iV, jV, kV ) = 0.0_KDR
          end if

        end do
      end do
    end do
    !$OMP end OMP_TARGET_DIRECTIVE parallel do simd
    
    !where ( AP_O + AM_O > 0.0_KDR )
    !  F_O = ( AP_O * RF_O  +  AM_O * cshift ( RF_I, shift = +1, dim = iD ) &
    !          - AP_O * AM_O * ( cshift ( U_I, shift = +1, dim = iD ) - U_O ) ) &
    !        / ( AP_O + AM_O )
    !elsewhere
    !  F_O = 0.0_KDR
    !end where
    
    iaS = 0
    iaS ( iD ) = +1
    
    !$OMP  OMP_TARGET_DIRECTIVE parallel do simd collapse ( 3 ) &
    !$OMP& schedule ( OMP_SCHEDULE_TARGET ) private ( iaVS )
    do kV = lV ( 3 ), uV ( 3 ) 
      do jV = lV ( 2 ), uV ( 2 )
        do iV = lV ( 1 ), uV ( 1 )
            
          iaVS = [ iV, jV, kV ] + iaS
          
          if ( AP_O ( iV, jV, kV ) + AM_O ( iV, jV, kV ) > 0.0_KDR ) then
            F_O ( iV, jV, kV ) &
              = ( AP_O ( iV, jV, kV ) * RF_O ( iV, jV, kV ) &
                  +  AM_O ( iV, jV, kV ) &
                     * RF_I ( iaVS ( 1 ), iaVS ( 2 ), iaVS ( 3 ) ) &
                  - AP_O ( iV, jV, kV ) * AM_O ( iV, jV, kV ) &
                    * ( U_I ( iaVS ( 1 ), iaVS ( 2 ), iaVS ( 3 ) ) &
                        - U_O ( iV, jV, kV ) ) ) &
                / ( AP_O ( iV, jV, kV ) + AM_O ( iV, jV, kV ) )
          else
            F_O ( iV, jV, kV ) = 0.0_KDR
          end if
          
        end do
      end do
    end do
    !$OMP end OMP_TARGET_DIRECTIVE parallel do simd
    
  end procedure ComputeFluxesKernel


void ComputeUpdate_C(double *dU, double *F_I, double *F_O,
        double V, double A, double dT, int nValues ) { 
    
    int iV;
    
    #pragma omp OMP_TARGET_DIRECTIVE parallel for simd
    {
        #pragma omp schedule ( OMP_SCHEDULE_TARGET )
        for (iV = 0, iV < size ( dU ), iV++) {
            dU ( iV ) = dU ( iV ) - dT * ( F_O ( iV ) - F_I ( iV ) ) * A / V
        }
    }
    
}
  
  
void AddUpdate_C( double *O, double *U, double *C, int nValues ) {
    
    int iV;
    
    nValues = sizeof( O );
    
    #pragma omp OMP_TARGET_DIRECTIVE parallel for simd
    {
        #pragma omp schedule ( OMP_SCHEDULE_TARGET )
        for (iV = 0, iV < nValues, iV++) {
            C ( iV ) = O ( iV ) + U ( iV );
        }
    }
    
}
  
  
void CombineUpdates_C(double *C, double *O, double *U, int nValues) {
    
    int iV;
    
    nValues = sizeof( O );
    
    #pragma omp OMP_TARGET_DIRECTIVE parallel for simd
    {
        #pragma omp schedule ( OMP_SCHEDULE_TARGET )
        for (iV = 0, iV < nValues; iV++) {
            C ( iV ) = 0.5 * ( O ( iV ) + ( C ( iV ) + U ( iV ) ) );
        }
    }
}
  

end submodule ConservationLawStep_Kernel
