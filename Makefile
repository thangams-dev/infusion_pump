all:
	cmake -B build -DCMAKE_CXX_FLAGS="--coverage"
	cmake --build build

coverage: all
	./build/infusion_test
	gcov -r build/CMakeFiles/infusion_test.dir/test_infusion.cpp.gcda
	lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
	lcov --remove coverage.info '*/bits/*' '*/gtest/*' --output-file coverage.info
	lcov --extract coverage.info '*/Infusion_pump/*' --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
clean:
	rm -rf build coverage.info coverage_report