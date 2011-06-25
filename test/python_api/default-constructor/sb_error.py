"""
Fuzz tests an object after the default construction to make sure it does not crash lldb.
"""

import sys
import lldb

def fuzz_obj(obj):
    obj.GetCString()
    obj.Fail()
    obj.Success()
    obj.GetError()
    obj.GetType()
    obj.SetError(5, lldb.eErrorTypeGeneric)
    obj.SetErrorToErrno()
    obj.SetErrorToGenericError()
    obj.SetErrorString("xyz")
    obj.SetErrorStringWithFormat("%s!", "error")
    obj.GetDescription(lldb.SBStream())