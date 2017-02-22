
source fpga/scripts/block_design.tcl

if {[version -short] >= 2016.3} {
  set_property synth_checkpoint_mode None [get_files $bd_path/system.bd]
}

generate_target all [get_files $bd_path/system.bd]
make_wrapper -files [get_files $bd_path/system.bd] -top

add_files -norecurse $bd_path/hdl/system_wrapper.v

# Add verilog source files
set files [glob -nocomplain $project_path/*.v $project_path/*.sv]
if {[llength $files] > 0} {
  add_files -norecurse $files
}

# Add constraint files
set fp [open tmp/$project_name.xdc r]
set files [split [read $fp]]

close $fp
if {[llength $files] > 0} {
  add_files -norecurse -fileset constrs_1 $files
}

set_property VERILOG_DEFINE {TOOL_VIVADO} [current_fileset]

switch $mode {
  "development" {
    puts "Running Vivado project in development mode"
    set_property STRATEGY Flow_RuntimeOptimized [get_runs synth_1]
    set_property STRATEGY Flow_RuntimeOptimized [get_runs impl_1]
  }
  "production" {
    puts "Running Vivado project in production mode"
    set_property STRATEGY Flow_PerfOptimized_High [get_runs synth_1]
    set_property STRATEGY Performance_NetDelay_high [get_runs impl_1]
  }
  default {
    puts "Warning : Unknown mode"
  }
}

close_project
