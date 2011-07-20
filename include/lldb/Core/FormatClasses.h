//===-- FormatClasses.h -----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef lldb_FormatClasses_h_
#define lldb_FormatClasses_h_

// C Includes

#include <stdint.h>
#include <unistd.h>

// C++ Includes
#include <string>

// Other libraries and framework includes

// Project includes
#include "lldb/lldb-public.h"
#include "lldb/lldb-enumerations.h"

#include "lldb/Core/ValueObject.h"
#include "lldb/Interpreter/ScriptInterpreterPython.h"
#include "lldb/Symbol/SymbolContext.h"

namespace lldb_private {

struct ValueFormat
{
    bool m_cascades;
    bool m_skip_pointers;
    bool m_skip_references;
    lldb::Format m_format;
    ValueFormat (lldb::Format f = lldb::eFormatInvalid,
                 bool casc = false,
                 bool skipptr = false,
                 bool skipref = false) : 
    m_cascades(casc),
    m_skip_pointers(skipptr),
    m_skip_references(skipref),
    m_format (f)
    {
    }
    
    typedef lldb::SharedPtr<ValueFormat>::Type SharedPointer;
    typedef bool(*ValueCallback)(void*, const char*, const ValueFormat::SharedPointer&);
    
    ~ValueFormat()
    {
    }
    
    bool
    Cascades()
    {
        return m_cascades;
    }
    bool
    SkipsPointers()
    {
        return m_skip_pointers;
    }
    bool
    SkipsReferences()
    {
        return m_skip_references;
    }
    
    lldb::Format
    GetFormat()
    {
        return m_format;
    }
    
    std::string
    FormatObject(lldb::ValueObjectSP object);
    
};

struct SummaryFormat
{
    bool m_cascades;
    bool m_skip_pointers;
    bool m_skip_references;
    bool m_dont_show_children;
    bool m_dont_show_value;
    bool m_show_members_oneliner;
    
    SummaryFormat(bool casc = false,
                  bool skipptr = false,
                  bool skipref = false,
                  bool nochildren = true,
                  bool novalue = true,
                  bool oneliner = false) :
    m_cascades(casc),
    m_skip_pointers(skipptr),
    m_skip_references(skipref),
    m_dont_show_children(nochildren),
    m_dont_show_value(novalue),
    m_show_members_oneliner(oneliner)
    {
    }
    
    bool
    Cascades()
    {
        return m_cascades;
    }
    bool
    SkipsPointers()
    {
        return m_skip_pointers;
    }
    bool
    SkipsReferences()
    {
        return m_skip_references;
    }
    
    bool
    DoesPrintChildren() const
    {
        return !m_dont_show_children;
    }
    
    bool
    DoesPrintValue() const
    {
        return !m_dont_show_value;
    }
    
    bool
    IsOneliner() const
    {
        return m_show_members_oneliner;
    }
            
    virtual
    ~SummaryFormat()
    {
    }
    
    virtual std::string
    FormatObject(lldb::ValueObjectSP object) = 0;
    
    virtual std::string
    GetDescription() = 0;
    
    typedef lldb::SharedPtr<SummaryFormat>::Type SharedPointer;
    typedef bool(*SummaryCallback)(void*, const char*, const SummaryFormat::SharedPointer&);
    typedef bool(*RegexSummaryCallback)(void*, lldb::RegularExpressionSP, const SummaryFormat::SharedPointer&);
    
};

// simple string-based summaries, using ${var to show data
struct StringSummaryFormat : public SummaryFormat
{
    std::string m_format;
    
    StringSummaryFormat(bool casc = false,
                        bool skipptr = false,
                        bool skipref = false,
                        bool nochildren = true,
                        bool novalue = true,
                        bool oneliner = false,
                        std::string f = "") :
    SummaryFormat(casc,skipptr,skipref,nochildren,novalue,oneliner),
    m_format(f)
    {
    }
    
    std::string
    GetFormat()
    {
        return m_format;
    }
    
    virtual
    ~StringSummaryFormat()
    {
    }
    
    virtual std::string
    FormatObject(lldb::ValueObjectSP object);
    
    virtual std::string
    GetDescription();
        
};
    
// Python-based summaries, running script code to show data
struct ScriptSummaryFormat : public SummaryFormat
{
    std::string m_function_name;
    std::string m_python_script;
    
    ScriptSummaryFormat(bool casc = false,
                        bool skipptr = false,
                        bool skipref = false,
                        bool nochildren = true,
                        bool novalue = true,
                        bool oneliner = false,
                        std::string fname = "",
                        std::string pscri = "") :
    SummaryFormat(casc,skipptr,skipref,nochildren,novalue,oneliner),
    m_function_name(fname),
    m_python_script(pscri)
    {
    }
    
    std::string
    GetFunctionName()
    {
        return m_function_name;
    }
    
    std::string
    GetPythonScript()
    {
        return m_python_script;
    }
    
    virtual
    ~ScriptSummaryFormat()
    {
    }
    
    virtual std::string
    FormatObject(lldb::ValueObjectSP object);
    
    virtual std::string
    GetDescription();
    
    typedef lldb::SharedPtr<ScriptSummaryFormat>::Type SharedPointer;

};

} // namespace lldb_private

#endif	// lldb_FormatClasses_h_