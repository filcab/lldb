"""Show bitfields and check that they display correctly."""

import os, time
import unittest2
import lldb
from lldbtest import *
import lldbutil

class FixAndContinueTestCase(TestBase):

    mydir = os.path.join("functionalities", "fix-and-continue")

    @unittest2.skipUnless(sys.platform.startswith("darwin"), "requires Darwin")
    @dsym_test
    def test_with_dsym_and_run_command(self):
        """Test 'frame variable ...' on a variable with bitfields."""
        self.buildDsym()
        self.fix_and_continue()

    @dwarf_test
    def test_with_dwarf_and_run_command(self):
        """Test 'frame variable ...' on a variable with bitfields."""
        self.buildDwarf()
        self.fix_and_continue()

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        # Find the line number to break inside main().
        self.line = line_number('main.c', '// Break here')

    def fix_and_continue(self):
        """Test 'target fix ...' functionality"""
        exe = os.path.join(os.getcwd(), "a.out")
        self.runCmd("file " + exe, CURRENT_EXECUTABLE_SET)

        # Break inside the main.
        lldbutil.run_break_set_by_file_and_line (self, "main.c", self.line, num_expected_locations=1, loc_exact=True)

        self.runCmd("run", RUN_SUCCEEDED)

        self.expect("next", "Got expected output",
            substrs = ['3'])

        self.expect("next", "Got expected output",
            substrs = ['9'])


if __name__ == '__main__':
    import atexit
    lldb.SBDebugger.Initialize()
    atexit.register(lambda: lldb.SBDebugger.Terminate())
    unittest2.main()
