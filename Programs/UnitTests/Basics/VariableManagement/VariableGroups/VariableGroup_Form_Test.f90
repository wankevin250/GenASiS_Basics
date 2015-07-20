program VariableGroup_Form_Test

  use Specifiers
  use ArrayArrays
  use VariableGroup_Form

  implicit none

  integer ( KDI ) :: &
    i
  character ( LDL ), dimension ( 6 ) :: &
    Variable &
      = [ 'Variable_1', 'Variable_2', 'Variable_3', 'Variable_4', &
          'Variable_5', 'Variable_6'  ], &
    UnitString &
      = [ 'Unit_1', 'Unit_2', 'Unit_3', 'Unit_4', 'Unit_5', 'Unit_6'  ]
  type ( MeasuredValueForm ), dimension ( 6 ) :: &
    VariableUnit
  type ( ArrayInteger_1D_Form ), dimension ( 1 ) :: &
    VectorIndices
  type ( VariableGroupForm ), dimension ( 3 ) :: &
    VG

  do i = 1, size ( VariableUnit ) 
    call VariableUnit ( i ) % Initialize &
           ( UnitString ( i ), UnitString ( i ), 1.0_KDR )
  end do

  call VectorIndices ( 1 ) % Initialize ( [ 2, 3 ] )

  !-- InitializeAllocate
  call VG ( 1 ) % Initialize &
         ( ValueShape = [ 4, 6 ], VectorIndicesOption = VectorIndices, &
           UnitOption = VariableUnit, VectorOption = [ 'Vector_1' ], &
           VariableOption = Variable, NameOption = 'VariableGroup_1')
  call PrintVariableGroup ( VG ( 1 ) )

  !-- InitializeClone
  call VG ( 2 ) % Initialize ( VG ( 1 ), NameOption = 'VariableGroup_2' )
  call PrintVariableGroup ( VG ( 2 ) )

  !-- InitializeClone, take 2
  call VG ( 3 ) % Initialize &
         ( VG ( 1 ), NameOption = 'VariableGroup_3', &
           SelectedOption = [ 2, 3, 6 ] )
  call PrintVariableGroup ( VG ( 3 ) )
  
contains


  subroutine PrintVariableGroup ( VG )

    use Specifiers
    use VariableGroup_Form

    integer ( KDI ) :: &
      i
    type ( VariableGroupForm ) :: &
      VG

    print *
    print *, 'VG % nValues = ', VG % nValues
    print *, 'VG % nVariables = ', VG % nVariables
    print *, 'VG % nVectors = ', VG % nVectors
    print *, 'VG % lName = ', VG % lName
    print *, 'VG % lVariable = ', VG % lVariable
    print *, 'VG % lVector = ', VG % lVector
    print *, 'VG % Selected = ', VG % Selected
    print *, 'VG % AllocatedValue = ', VG % AllocatedValue
    print *, 'VG % Name = ', trim ( VG % Name )

    do i = 1, VG % nVariables
      print *, &
        'VG % Variable (', i, ') = ', &
        trim ( VG % Variable ( VG % Selected ( i ) ) )
    end do

    do i = 1, VG % nVectors
      print *, &
        'VG % Vector (', i, ') = ', trim ( VG % Vector ( i ) )
    end do

    do i = 1, VG % nVariables
      print *, &
        'VG % Unit (', i, ') % Unit = ', &
        trim ( VG % Unit ( VG % Selected ( i ) ) % Unit )
    end do

    do i = 1, VG % nVectors
      print *, &
        'VG % VectorIndices (', i, ') = ', VG % VectorIndices ( i ) % Value
    end do

  end subroutine PrintVariableGroup


end program VariableGroup_Form_Test

