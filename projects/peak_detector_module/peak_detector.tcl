namespace eval peak_detector {

proc pins {cmd wfm_width} {
  $cmd -dir I -from 31 -to 0                   din
  $cmd -dir I -from [expr $wfm_width -1] -to 0 address_low
  $cmd -dir I -from [expr $wfm_width -1] -to 0 address_high
  $cmd -dir I -from [expr $wfm_width -1] -to 0 address_reset
  $cmd -dir I                                  s_axis_tvalid
  $cmd -dir O -from [expr $wfm_width -1] -to 0 address_out
  $cmd -dir O -from 31 -to 0                   maximum_out
  $cmd -dir O                                  m_axis_tvalid
  $cmd -dir I -type clk                        clk
}

proc create {module_name wfm_width} {

  set bd [current_bd_instance .]
  current_bd_instance [create_bd_cell -type hier $module_name]

  pins create_bd_pin $wfm_width

  set compare_latency 0

  # Add comparator
  cell xilinx.com:ip:floating_point:7.1 comparator {
    Operation_Type Compare
    C_Compare_Operation Greater_Than
    Flow_Control NonBlocking
    Maximum_Latency False
    C_Latency $compare_latency
  } {
    s_axis_a_tdata din
    s_axis_a_tvalid s_axis_tvalid
    s_axis_b_tvalid s_axis_tvalid
  }

  # Address starting counting at s_axis_tvalid
  cell xilinx.com:ip:c_counter_binary:12.0 address_counter {
    CE true
    Output_Width $wfm_width
  } {
    CLK clk
    CE s_axis_tvalid
  }

  # OR
  cell xilinx.com:ip:util_vector_logic:2.0 logic_or {
    C_SIZE 1
    C_OPERATION or
  } {
    Op2 [get_EQ_pin $wfm_width address_counter/Q address_reset]
  }

  # Register storing the current maximum
  cell xilinx.com:ip:c_shift_ram:12.0 maximum_reg {
    CE true
    Width 32
    Depth 1
  } {
    CLK clk
    D din
    CE logic_or/Res
    Q comparator/s_axis_b_tdata
  }

  # Register storing the maximum of one cycle
  cell xilinx.com:ip:c_shift_ram:12.0 maximum_out {
    CE true
    Width 32
    Depth 1
  } {
    CLK clk
    CE reset_cycle/dout
    D [get_Q_pin din 32 1 logic_or/Res]
    Q maximum_out
  }

  # Register storing the address of the maximum of one cycle
  cell xilinx.com:ip:c_shift_ram:12.0 address_out {
    CE true
    Width $wfm_width
    Depth 1
  } {
    CLK clk
    CE reset_cycle/Dout
    D [get_Q_pin address_counter/Q $wfm_width 1 logic_or/Res]
    Q address_out
  }

  # Restrict peak detection between address_low and address_high
  cell xilinx.com:ip:util_vector_logic:2.0 maximum_detected_in_range {
    C_SIZE 1
    C_OPERATION and
  } {
    Op1 [get_and_pin \
          [get_GE_pin $wfm_width address_counter/Q address_low] \
          [get_LE_pin $wfm_width address_counter/Q address_high] \
        ]
    Op2 [get_slice_pin comparator/m_axis_result_tdata 8 0 0]
    Res logic_or/Op1
  }
  
  connect_pins m_axis_tvalid [get_Q_pin reset_cycle/dout 32 1]

  current_bd_instance $bd
}

} ;# end peak detector namespace
