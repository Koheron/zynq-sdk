set -e

echo 'Test config.yml'
python scripts/make.py --test

# Test modules

for module in address averager peak_detector spectrum
do
	echo 'Test config.yml for module' $module
	python scripts/make.py --split_config_yml fpga/modules/$module
done

# Test instrument drivers

for instrument in led_blinker adc_dac laser_controller pulse_generator oscillo spectrum test_context zedboard_led_blinker
do
    make NAME=$instrument server
done
