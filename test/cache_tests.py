import unittest
import subprocess
import os

class CacheProgramTests(unittest.TestCase):
    def setUp(self):
        self.binary = "./cache"

        self.valid_file = "test/inputs/valid_data.csv"
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
            "test/inputs/invalid_data15.csv"
        ]
        self.nonexistent_file = "test/inputs/does_not_exist.csv"

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

    def test_set_cacheline_and_levels(self):
        result = self.run_cache([
            "--debug",
            "-C", "64",
            "-L", "128",
            "-M", "256",
            "-N", "512",
            "-l", "1",
            "-m", "2",
            "-n", "3",
            "-e", "3",
            "-S", "1",
            self.valid_file
        ])
        self.assertEqual(result.returncode, 0)

    def test_negative_cycles(self):
        result = self.run_cache([
            "-d",
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


    def test_no_input_file(self):
        result = self.run_cache([
            "-c", "100",
        ])
        self.assertIn("No input", result.stderr)
        self.assertNotEqual(result.returncode, 0)

if __name__ == '__main__':
    unittest.main()
