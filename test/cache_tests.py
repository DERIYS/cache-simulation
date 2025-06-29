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
            "test/inputs/invalid_data12.csv"
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
        self.assertIn("Request", result.stdout)
        self.assertIn("Success", result.stdout)

    def test_invalid_csv_files(self):
        for invalid_file in self.invalid_files:
            with self.subTest(invalid_file=invalid_file):
                result = self.run_cache([invalid_file])
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Failed to parse CSV", result.stdout)

    def test_missing_csv_file(self):
        result = self.run_cache([self.nonexistent_file])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Access denied", result.stdout)

    def test_help_option(self):
        result = self.run_cache(["-help"])
        self.assertEqual(result.returncode, 0)
        self.assertIn("Help", result.stdout)

    def test_invalid_cycles_option(self):
        result = self.run_cache(["-c", "0", self.valid_file])
        self.assertNotEqual(result.returncode, 0)

    def test_set_cacheline_and_levels(self):
        result = self.run_cache([
            "-C", "64",
            "-1", "128",
            "-2", "256",
            "-3", "512",
            "-l", "1",
            "-a", "2",
            "-t", "3",
            "-L", "3",
            "-S", "1",
            self.valid_file
        ])
        self.assertEqual(result.returncode, 0)
        self.assertIn("Cacheline size set", result.stdout)
        self.assertIn("Cache L1 lines set", result.stdout)
        self.assertIn("Cache L2 lines set", result.stdout)
        self.assertIn("Cache L3 lines set", result.stdout)
        self.assertIn("Cache L1 latency set", result.stdout)
        self.assertIn("Cache L2 latency set", result.stdout)
        self.assertIn("Cache L3 latency set", result.stdout)
        self.assertIn("Cache levels set", result.stdout)
        self.assertIn("Mapping strategy set", result.stdout)
        self.assertIn("Success", result.stdout)

    def test_no_input_file(self):
        result = self.run_cache([])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("No input file", result.stdout)
    
if __name__ == '__main__':
    unittest.main()
