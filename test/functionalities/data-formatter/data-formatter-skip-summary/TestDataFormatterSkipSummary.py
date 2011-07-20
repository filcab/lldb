"""
Test lldb data formatter subsystem.
"""

import os, time
import unittest2
import lldb
from lldbtest import *

class DataFormatterTestCase(TestBase):

    mydir = os.path.join("functionalities", "data-formatter", "data-formatter-skip-summary")

    @unittest2.skipUnless(sys.platform.startswith("darwin"), "requires Darwin")
    def test_with_dsym_and_run_command(self):
        """Test data formatter commands."""
        self.buildDsym()
        self.data_formatter_commands()

    def test_with_dwarf_and_run_command(self):
        """Test data formatter commands."""
        self.buildDwarf()
        self.data_formatter_commands()

    def setUp(self):
        # Call super's setUp().
        TestBase.setUp(self)
        # Find the line number to break at.
        self.line = line_number('main.cpp', '// Set break point at this line.')

    def data_formatter_commands(self):
        """Test that that file and class static variables display correctly."""
        self.runCmd("file a.out", CURRENT_EXECUTABLE_SET)

        self.expect("breakpoint set -f main.cpp -l %d" % self.line,
                    BREAKPOINT_CREATED,
            startstr = "Breakpoint created: 1: file ='main.cpp', line = %d, locations = 1" %
                        self.line)

        self.runCmd("run", RUN_SUCCEEDED)

        # The stop reason of the thread should be breakpoint.
        self.expect("thread list", STOPPED_DUE_TO_BREAKPOINT,
            substrs = ['stopped',
                       'stop reason = breakpoint'])

        # This is the function to remove the custom formats in order to have a
        # clean slate for the next test case.
        def cleanup():
            self.runCmd('type format clear', check=False)
            self.runCmd('type summary clear', check=False)

        # Execute the cleanup function during test case tear down.
        self.addTearDownHook(cleanup)

        # Setup the summaries for this scenario
        self.runCmd("type summary add -f \"${var._M_dataplus._M_p}\" std::string") # This works fine on OSX 10.6.8, if it differs on your implementation, submit a patch to adapt it to your C++ stdlib
        self.runCmd("type summary add -f \"Level 1\" \"DeepData_1\"")
        self.runCmd("type summary add -f \"Level 2\" \"DeepData_2\" -e")
        self.runCmd("type summary add -f \"Level 3\" \"DeepData_3\"")
        self.runCmd("type summary add -f \"Level 4\" \"DeepData_4\"")
        self.runCmd("type summary add -f \"Level 5\" \"DeepData_5\"")
            
        # Default case, just print out summaries
        self.expect('frame variable',
            substrs = ['(DeepData_1) data1 = Level 1',
                       '(DeepData_2) data2 = Level 2 {',
                       'm_child1 = Level 3',
                       'm_child2 = Level 3',
                       'm_child3 = Level 3',
                       'm_child4 = Level 3',
                       '}'])

        # Skip the default (should be 1) levels of summaries
        self.expect('frame variable -Y',
            substrs = ['(DeepData_1) data1 = {',
                       'm_child1 = 0x',
                       '}',
                       '(DeepData_2) data2 = {',
                       'm_child1 = Level 3',
                       'm_child2 = Level 3',
                       'm_child3 = Level 3',
                       'm_child4 = Level 3',
                       '}'])

        # Now skip 2 levels of summaries
        self.expect('frame variable -Y2',
            substrs = ['(DeepData_1) data1 = {',
                       'm_child1 = 0x',
                       '}',
                       '(DeepData_2) data2 = {',
                       'm_child1 = {',
                       'm_child1 = 0x',
                       'Level 4',
                       'm_child2 = {',
                       'm_child3 = {',
                       '}'])

        # Check that no "Level 3" comes out
        self.expect('frame variable data1.m_child1 -Y2', matching=False,
            substrs = ['Level 3'])

        # Now expand a pointer with 2 level of skipped summaries
        self.expect('frame variable data1.m_child1 -Y2',
                    substrs = ['(DeepData_2 *) data1.m_child1 = 0x'])

        # Deref and expand said pointer
        self.expect('frame variable *data1.m_child1 -Y2',
                    substrs = ['(DeepData_2) *data1.m_child1 = {',
                               'm_child2 = {',
                               'm_child1 = 0x',
                               'Level 4',
                               '}'])

        # Expand an expression, skipping 2 layers of summaries
        self.expect('frame variable data1.m_child1->m_child2 -Y2',
                substrs = ['(DeepData_3) data1.m_child1->m_child2 = {',
                           'm_child2 = {',
                           'm_child1 = Level 5',
                           'm_child2 = Level 5',
                           'm_child3 = Level 5',
                           '}'])

        # Expand same expression, skipping only 1 layer of summaries
        self.expect('frame variable data1.m_child1->m_child2 -Y1',
                    substrs = ['(DeepData_3) data1.m_child1->m_child2 = {',
                               'm_child1 = 0x',
                               'Level 4',
                               'm_child2 = Level 4',
                               '}'])

        # Expand same expression, skipping 3 layers of summaries
        self.expect('frame variable data1.m_child1->m_child2 -T -Y3',
                    substrs = ['(DeepData_3) data1.m_child1->m_child2 = {',
                               'm_some_text = "Just a test"',
                               'm_child2 = {',
                               'm_some_text = "Just a test"'])

        # Expand within a standard string (might depend on the implementation of the C++ stdlib you use)
        self.expect('frame variable data1.m_child1->m_child2.m_child1.m_child2 -Y2',
            substrs = ['(DeepData_5) data1.m_child1->m_child2.m_child1.m_child2 = {',
                       'm_some_text = {',
                       '_M_dataplus = {',
                       '_M_p = 0x',
                       '"Just a test"'])

        # Repeat the above, but only skip 1 level of summaries
        self.expect('frame variable data1.m_child1->m_child2.m_child1.m_child2 -Y1',
                    substrs = ['(DeepData_5) data1.m_child1->m_child2.m_child1.m_child2 = {',
                               'm_some_text = "Just a test"',
                               '}'])

        # Change summary and expand, first without -Y then with -Y
        self.runCmd("type summary add -f \"${var.m_some_text}\" DeepData_5")
        
        self.expect('fr var data2.m_child4.m_child2.m_child2',
            substrs = ['(DeepData_5) data2.m_child4.m_child2.m_child2 = "Just a test"'])

        self.expect('fr var data2.m_child4.m_child2.m_child2 -Y',
                    substrs = ['(DeepData_5) data2.m_child4.m_child2.m_child2 = {',
                               'm_some_text = "Just a test"',
                               '}'])


if __name__ == '__main__':
    import atexit
    lldb.SBDebugger.Initialize()
    atexit.register(lambda: lldb.SBDebugger.Terminate())
    unittest2.main()