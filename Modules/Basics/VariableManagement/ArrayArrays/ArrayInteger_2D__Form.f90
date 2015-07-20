!-- ArrayInteger_2D_Form allows the construction of an array of 2D integer
!   arrays to form ragged arrays.

module ArrayInteger_2D__Form

  use Specifiers
  use ArrayOperations

  implicit none
  private

  type, public :: ArrayInteger_2D_Form
    integer ( KDI ), dimension ( :, : ), allocatable :: &
      Value
  contains
    procedure, private, pass :: &
      Initialize_AI_2D
    procedure, private, pass :: &
      Initialize_AI_2D_FromValue
    procedure, private, pass :: &
      Initialize_AI_2D_Copy
    generic :: &
      Initialize &
        => Initialize_AI_2D, Initialize_AI_2D_FromValue, Initialize_AI_2D_Copy
    final :: &
      Finalize_AI_2D
  end type ArrayInteger_2D_Form
  
contains


  subroutine Initialize_AI_2D ( A, nValues, ClearOption, iaLowerBoundOption )
    
    class ( ArrayInteger_2D_Form ), intent ( inout ) :: &
      A
    integer ( KDI ), dimension ( 2 ), intent ( in ) :: &
      nValues
    logical ( KDL ), intent ( in ), optional :: &
      ClearOption
    integer ( KDI ), dimension ( 2 ), intent ( in ), optional :: &
      iaLowerBoundOption

    integer ( KDI ), dimension ( 2 ) :: &
      iaLB
    logical ( KDL ) :: &
      ClearRequested

    if ( any ( nValues < 0 ) ) return
    
    if ( all ( nValues == 0 ) ) then
      allocate ( A % Value ( 0, 0 ) )
      return
    end if 
    
    ClearRequested = .false.
    if ( present ( ClearOption ) ) ClearRequested = ClearOption

    iaLB = 1
    if ( present ( iaLowerBoundOption ) ) iaLB = iaLowerBoundOption
    
    allocate &
      ( A % Value &
          ( iaLB ( 1 ) : iaLB ( 1 ) + nValues ( 1 ) - 1, &
            iaLB ( 2 ) : iaLB ( 2 ) + nValues ( 2 ) - 1 ) )
    
    if ( ClearRequested ) call Clear ( A % Value )

  end subroutine Initialize_AI_2D
  
  
  subroutine Initialize_AI_2D_FromValue ( A, Value, iaLowerBoundOption )
    
    class ( ArrayInteger_2D_Form ), intent ( inout ) :: &
      A
    integer ( KDI ), dimension ( :, : ), intent ( in ) :: &
      Value
    integer ( KDI ), dimension ( 2 ), intent ( in ), optional :: &
      iaLowerBoundOption

    call A % Initialize_AI_2D &
           ( shape ( Value ), iaLowerBoundOption = iaLowerBoundOption )
    A % Value = Value 

  end subroutine Initialize_AI_2D_FromValue
  
  
  subroutine Initialize_AI_2D_Copy ( A, B, iaLowerBoundOption )
    
    class ( ArrayInteger_2D_Form ), intent ( inout ) :: &
      A
    type (  ArrayInteger_2D_Form ), intent ( in ) :: &
      B
    integer ( KDI ), intent ( in ), optional :: &
      iaLowerBoundOption
      
    integer ( KDI ), dimension ( 2 ) :: &
      iaLB
    
    iaLB = lbound ( B % Value ) 
    if ( present ( iaLowerBoundOption ) ) iaLB = iaLowerBoundOption

    call A % Initialize_AI_2D_FromValue ( B % Value, iaLowerBoundOption = iaLB )
  
  end subroutine Initialize_AI_2D_Copy 
  
  
  elemental subroutine Finalize_AI_2D ( A )

    type ( ArrayInteger_2D_Form ), intent ( inout ) :: &
      A

    if ( allocated ( A % Value ) ) deallocate ( A % Value )

  end subroutine Finalize_AI_2D
  
  
end module ArrayInteger_2D__Form
