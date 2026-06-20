all:
	cmake -B build -DCMAKE_CXX_FLAGS="--coverage"
	cmake --build build

coverage: all
	./build/infusion_test
	lcov --capture --directory build --output-file coverage.info --ignore-errors mismatch
	lcov --remove coverage.info '*/bits/*' '*/gtest/*' '/usr/*' --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
clean:
	rm -rf build coverage.info coverage_report