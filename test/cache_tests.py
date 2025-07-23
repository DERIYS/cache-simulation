import unittest
import subprocess
import os

class CacheProgramTests(unittest.TestCase):
    def setUp(self):
        self.binary = "./project"

        self.valid_file = "test/inputs/valid_data.csv"
        self.expected_values_file = "test/inputs/expected_values.csv"
        self.invalid_files = [
            "test/inputs/invalid_data1.csv",
            "test/inputs/invalid_data2.csv",
            "test/inputs/invalid_data3.csv",
            "test/inputs/invalid_data4.csv",
            "test/inputs/invalid_data5.csv",
            "test/inputs/invalid_data6.csv",
            "test/inputs/invalid_data7.csv",
            "test/inputs/invalid_data8.csv",
            "test/inputs/invalid_data9.csv",
            "test/inputs/invalid_data10.csv",
            "test/inputs/invalid_data11.csv",
            "test/inputs/invalid_data12.csv",
            "test/inputs/invalid_data13.csv",
            "test/inputs/invalid_data14.csv",
            "test/inputs/invalid_data15.csv",
            "test/inputs/invalid_data16.csv",
            "test/inputs/restricted.csv"
        ]
        self.nonexistent_file = "test/inputs/does_not_exist.csv"

        self.flags = [
            "-c",
            "-C",
            "-L",
            "-M",
            "-N",
            "-l",
            "-m",
            "-n",
            "-e",
            "-S",
        ]

    def run_cache(self, args):
        return subprocess.run(
            [self.binary] + args,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

    def test_valid_csv_file(self):
        result = self.run_cache([self.valid_file])
        self.assertEqual(result.returncode, 0)

    def test_invalid_csv_files(self):
        for invalid_file in self.invalid_files:
            with self.subTest(invalid_file=invalid_file):
                result = self.run_cache([invalid_file])
                self.assertNotEqual(result.returncode, 0)

    def test_missing_csv_file(self):
        result = self.run_cache([self.nonexistent_file])
        self.assertNotEqual(result.returncode, 0)

    def test_help_option(self):
        result = self.run_cache(["-help"])
        self.assertEqual(result.returncode, 0)

    def test_invalid_cycles_option(self):
        result = self.run_cache(["-c", "0", self.valid_file])
        self.assertNotEqual(result.returncode, 0)

    def test_set_all(self):
        result = self.run_cache([
            "-d",
            "-f", "tracefile",
            "-C", "64",
            "-L", "128",
            "-M", "256",
            "-N", "512",
            "-l", "1",
            "-m", "2",
            "-n", "3",
            "-e", "3",
            "-S", "1",
            "-t",
            self.expected_values_file
        ])
        self.assertEqual(result.returncode, 0)

    def test_negative_cycles(self):
        result = self.run_cache([
            "-d",
            "-t",
            "-c", "-1000",
            self.valid_file
        ])
        self.assertIn("Negative", result.stderr)
        self.assertNotEqual(result.returncode,0)

    def test_literals_cycles(self):
        result = self.run_cache([
            "-c", "1000a",
            self.valid_file
        ])
        self.assertIn("Literals", result.stderr)
        self.assertNotEqual(result.returncode,0)

    def test_overflow_cycles(self):
        result = self.run_cache([
            "-c", "100000000000000000000000000000000000000000000000",
            self.valid_file
        ])        
        self.assertIn("Buffer error", result.stderr)
        self.assertNotEqual(result.returncode,0)

    def test_linesize_not_power_of_2(self):
        result = self.run_cache([
            "-C", "7",
            self.valid_file
        ])
        self.assertIn("is not a power of 2", result.stderr)
        self.assertNotEqual(result.returncode, 0)
    
    def test_false_cache_levels(self):
        result = self.run_cache([
            "-e", "4",
            self.valid_file
        ])
        self.assertIn("is between 1 and 3", result.stderr)
        self.assertNotEqual(result.returncode, 0)

    def test_false_mapping_strategy(self):
        result = self.run_cache([
            "-S", "2",
            self.valid_file
        ])
        self.assertIn("Mapping strategy", result.stderr)
        self.assertNotEqual(result.returncode, 0)

    def test_invalid_input_for_each_flag(self):
        for flag in self.flags:
            with self.subTest(flag=flag):
                result = self.run_cache([flag, "10a", self.valid_file])
                self.assertNotEqual(result.returncode, 0)

    def test_invalid_flag(self):
        result = self.run_cache([
            "-P", "2",
            self.valid_file
        ])

        self.assertNotEqual(result.returncode, 0)

    def test_direct_mapped(self):
        result = self.run_cache([
            "-S", "0",
            self.valid_file
        ])
        self.assertEqual(result.returncode,0)

    def test_no_input_file(self):
        result = self.run_cache([
            "-c", "100",
        ])
        self.assertIn("No input", result.stderr)
        self.assertNotEqual(result.returncode, 0)


if __name__ == '__main__':
    unittest.main()
