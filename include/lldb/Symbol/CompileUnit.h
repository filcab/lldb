//===-- CompileUnit.h -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_CompUnit_h_
#define liblldb_CompUnit_h_

#include "lldb/lldb-enumerations.h"
#include "lldb/Symbol/Function.h"
#include "lldb/Core/FileSpecList.h"
#include "lldb/Core/ModuleChild.h"
#include "lldb/Core/Stream.h"
#include "lldb/Core/UserID.h"

namespace lldb_private {
//----------------------------------------------------------------------
/// @class CompileUnit CompileUnit.h "lldb/Symbol/CompileUnit.h"
/// @brief A class that describes a compilation unit.
///
/// A representation of a compilation unit, or compiled source file.
/// The UserID of the compile unit is specified by the SymbolFile
/// plug-in and can have any value as long as the value is unique
/// within the Module that owns this compile units.
///
/// Each compile unit has a list of functions, global and static
/// variables, support file list (include files and inlined source
/// files), and a line table.
//----------------------------------------------------------------------
class CompileUnit :
    public STD_ENABLE_SHARED_FROM_THIS(CompileUnit),
    public ModuleChild,
    public FileSpec,
    public UserID,
    public SymbolContextScope
{
public:
    //------------------------------------------------------------------
    /// Construct with a module, path, UID and language.
    ///
    /// Initialize the compile unit given the owning \a module, a path
    /// to convert into a FileSpec, the SymbolFile plug-in supplied
    /// \a uid, and the source language type.
    ///
    /// @param[in] module
    ///     The parent module that owns this compile unit. This value
    ///     must be a valid pointer value.
    ///
    /// @param[in] user_data
    ///     User data where the SymbolFile parser can store data.
    ///
    /// @param[in] pathname
    ///     The path to the source file for this compile unit.
    ///
    /// @param[in] uid
    ///     The user ID of the compile unit. This value is supplied by
    ///     the SymbolFile plug-in and should be a value that allows
    ///     the SymbolFile plug-in to easily locate and parse additional
    ///     information for the compile unit.
    ///
    /// @param[in] language
    ///     A language enumeration type that describes the main language
    ///     of this compile unit.
    ///
    /// @see lldb::LanguageType
    //------------------------------------------------------------------
    CompileUnit(const lldb::ModuleSP &module_sp, void *user_data, const char *pathname, lldb::user_id_t uid, lldb::LanguageType language);

    //------------------------------------------------------------------
    /// Construct with a module, file spec, UID and language.
    ///
    /// Initialize the compile unit given the owning \a module, a path
    /// to convert into a FileSpec, the SymbolFile plug-in supplied
    /// \a uid, and the source language type.
    ///
    /// @param[in] module
    ///     The parent module that owns this compile unit. This value
    ///     must be a valid pointer value.
    ///
    /// @param[in] user_data
    ///     User data where the SymbolFile parser can store data.
    ///
    /// @param[in] file_spec
    ///     The file specification for the source file of this compile
    ///     unit.
    ///
    /// @param[in] uid
    ///     The user ID of the compile unit. This value is supplied by
    ///     the SymbolFile plug-in and should be a value that allows
    ///     the plug-in to easily locate and parse
    ///     additional information for the compile unit.
    ///
    /// @param[in] language
    ///     A language enumeration type that describes the main language
    ///     of this compile unit.
    ///
    /// @see lldb::LanguageType
    //------------------------------------------------------------------
    CompileUnit(const lldb::ModuleSP &module_sp, void *user_data, const FileSpec &file_spec, lldb::user_id_t uid, lldb::LanguageType language);

    //------------------------------------------------------------------
    /// Destructor
    //------------------------------------------------------------------
    virtual
    ~CompileUnit();

    //------------------------------------------------------------------
    /// Add a function to this compile unit.
    ///
    /// Typically called by the SymbolFile plug-ins as they partially
    /// parse the debug information.
    ///
    /// @param[in] function_sp
    ///     A shared pointer to the a Function object.
    //------------------------------------------------------------------
    void
    AddFunction(lldb::FunctionSP& function_sp);

    //------------------------------------------------------------------
    /// @copydoc SymbolContextScope::CalculateSymbolContext(SymbolContext*)
    ///
    /// @see SymbolContextScope
    //------------------------------------------------------------------
    virtual void
    CalculateSymbolContext(SymbolContext* sc);

    virtual lldb::ModuleSP
    CalculateSymbolContextModule ();
    
    virtual CompileUnit *
    CalculateSymbolContextCompileUnit ();

    //------------------------------------------------------------------
    /// @copydoc SymbolContextScope::DumpSymbolContext(Stream*)
    ///
    /// @see SymbolContextScope
    //------------------------------------------------------------------
    virtual void
    DumpSymbolContext(Stream *s);

    lldb::LanguageType
    GetLanguage();

    void
    SetLanguage(lldb::LanguageType language)
    {
        m_flags.Set(flagsParsedLanguage);
        m_language = language;
    }

    void
    GetDescription(Stream *s, lldb::DescriptionLevel level) const;

    //------------------------------------------------------------------
    /// Get a shared pointer to a function in this compile unit by
    /// index.
    ///
    /// Typically called when iterating though all functions in a
    /// compile unit after all functions have been parsed. This provides
    /// raw access to the function shared pointer list and will not
    /// cause the SymbolFile plug-in to parse any unparsed functions.
    ///
    /// @param[in] idx
    ///     An index into the function list.
    ///
    /// @return
    ///     A shared pointer to a function that might contain a NULL
    ///     Function class pointer.
    //------------------------------------------------------------------
    lldb::FunctionSP
    GetFunctionAtIndex (size_t idx);

    //------------------------------------------------------------------
    /// Dump the compile unit contents to the stream \a s.
    ///
    /// @param[in] s
    ///     The stream to which to dump the object descripton.
    ///
    /// @param[in] show_context
    ///     If \b true, variables will dump their symbol context
    ///     information.
    //------------------------------------------------------------------
    void
    Dump (Stream *s, bool show_context) const;

    //------------------------------------------------------------------
    /// Find the line entry by line and optional inlined file spec.
    ///
    /// Finds the first line entry that has an index greater than
    /// \a start_idx that matches \a line. If \a file_spec_ptr
    /// is NULL, then the search matches line entries whose file matches
    /// the file for the compile unit. If \a file_spec_ptr is
    /// not NULL, line entries must match the specified file spec (for
    /// inlined line table entries).
    ///
    /// Multiple calls to this function can find all entries that match
    /// a given file and line by starting with \a start_idx equal to zero,
    /// and calling this function back with the return valeu + 1.
    ///
    /// @param[in] start_idx
    ///     The zero based index at which to start looking for matches.
    ///
    /// @param[in] line
    ///     The line number to search for.
    ///
    /// @param[in] file_spec_ptr
    ///     If non-NULL search for entries that match this file spec,
    ///     else if NULL, search for line entries that match the compile
    ///     unit file.
    ///
    /// @param[in] exact
    ///     If \btrue match only if there is a line table entry for this line number.
    ///     If \bfalse, find the line table entry equal to or after this line number.
    ///
    /// @param[out] line_entry
    ///     If non-NULL, a copy of the line entry that was found.
    ///
    /// @return
    ///     The zero based index of a matching line entry, or UINT32_MAX
    ///     if no matching line entry is found.
    //------------------------------------------------------------------
    uint32_t
    FindLineEntry (uint32_t start_idx,
                   uint32_t line,
                   const FileSpec* file_spec_ptr,
                   bool exact,
                   LineEntry *line_entry);

    //------------------------------------------------------------------
    /// Get the line table for the compile unit.
    ///
    /// Called by clients and the SymbolFile plug-in. The SymbolFile
    /// plug-ins use this function to determine if the line table has
    /// be parsed yet. Clients use this function to get the line table
    /// from a compile unit.
    ///
    /// @return
    ///     The line table object pointer, or NULL if this line table
    ///     hasn't been parsed yet.
    //------------------------------------------------------------------
    LineTable*
    GetLineTable ();

    //------------------------------------------------------------------
    /// Get the compile unit's support file list.
    ///
    /// The support file list is used by the line table, and any objects
    /// that have valid Declaration objects.
    ///
    /// @return
    ///     A support file list object.
    //------------------------------------------------------------------
    FileSpecList&
    GetSupportFiles ();

    //------------------------------------------------------------------
    /// Get the SymbolFile plug-in user data.
    ///
    /// SymbolFile plug-ins can store user data to internal state or
    /// objects to quickly allow them to parse more information for a
    /// given object.
    ///
    /// @return
    ///     The user data stored with the CompileUnit when it was
    ///     constructed.
    //------------------------------------------------------------------
    void *
    GetUserData () const;

    //------------------------------------------------------------------
    /// Get the variable list for a compile unit.
    ///
    /// Called by clients to get the variable list for a compile unit.
    /// The variable list will contain all global and static variables
    /// that were defined at the compile unit level.
    ///
    /// @param[in] can_create
    ///     If \b true, the variable list will be parsed on demand. If
    ///     \b false, the current variable list will be returned even
    ///     if it contains a NULL VariableList object (typically
    ///     called by dumping routines that want to display only what
    ///     has currently been parsed).
    ///
    /// @return
    ///     A shared pointer to a variable list, that can contain NULL
    ///     VariableList pointer if there are no global or static
    ///     variables.
    //------------------------------------------------------------------
    lldb::VariableListSP
    GetVariableList (bool can_create);

    //------------------------------------------------------------------
    /// Finds a function by user ID.
    ///
    /// Typically used by SymbolFile plug-ins when partially parsing
    /// the debug information to see if the function has been parsed
    /// yet.
    ///
    /// @param[in] uid
    ///     The user ID of the function to find. This value is supplied
    ///     by the SymbolFile plug-in and should be a value that
    ///     allows the plug-in to easily locate and parse additional
    ///     information in the function.
    ///
    /// @return
    ///     A shared pointer to the function object that might contain
    ///     a NULL Function pointer.
    //------------------------------------------------------------------
    lldb::FunctionSP
    FindFunctionByUID (lldb::user_id_t uid);

    //------------------------------------------------------------------
    /// Set the line table for the compile unit.
    ///
    /// Called by the SymbolFile plug-in when if first parses the line
    /// table and hands ownership of the line table to this object. The
    /// compile unit owns the line table object and will delete the
    /// object when it is deleted.
    ///
    /// @param[in] line_table
    ///     A line table object pointer that this object now owns.
    //------------------------------------------------------------------
    void
    SetLineTable(LineTable* line_table);

    //------------------------------------------------------------------
    /// Set accessor for the variable list.
    ///
    /// Called by the SymbolFile plug-ins after they have parsed the
    /// variable lists and are ready to hand ownership of the list over
    /// to this object.
    ///
    /// @param[in] variable_list_sp
    ///     A shared pointer to a VariableList.
    //------------------------------------------------------------------
    void
    SetVariableList (lldb::VariableListSP& variable_list_sp);

    //------------------------------------------------------------------
    /// Resolve symbol contexts by file and line.
    ///
    /// Given a file in \a file_spec, and a line number, find all
    /// instances and append them to the supplied symbol context list
    /// \a sc_list.
    ///
    /// @param[in] file_spec
    ///     A file specification. If \a file_spec contains no directory
    ///     information, only the basename will be used when matching
    ///     contexts. If the directory in \a file_spec is valid, a
    ///     complete file specification match will be performed.
    ///
    /// @param[in] line
    ///     The line number to match against the compile unit's line
    ///     tables.
    ///
    /// @param[in] check_inlines
    ///     If \b true this function will also match any inline
    ///     file and line matches. If \b false, the compile unit's
    ///     file specification must match \a file_spec for any matches
    ///     to be returned.
    ///
    /// @param[in] exact
    ///     If true, only resolve the context if \a line exists in the line table.
    ///     If false, resolve the context to the closest line greater than \a line
    ///     in the line table.
    ///
    /// @param[in] resolve_scope
    ///     For each matching line entry, this bitfield indicates what
    ///     values within each SymbolContext that gets added to \a
    ///     sc_list will be resolved. See the SymbolContext::Scope
    ///     enumeration for a list of all available bits that can be
    ///     resolved. Only SymbolContext entries that can be resolved
    ///     using a LineEntry base address will be able to be resolved.
    ///
    /// @param[out] sc_list
    ///     A SymbolContext list class that willl get any matching
    ///     entries appended to.
    ///
    /// @return
    ///     The number of new matches that were added to \a sc_list.
    ///
    /// @see enum SymbolContext::Scope
    //------------------------------------------------------------------
    uint32_t
    ResolveSymbolContext (const FileSpec& file_spec,
                          uint32_t line,
                          bool check_inlines,
                          bool exact,
                          uint32_t resolve_scope,
                          SymbolContextList &sc_list);


protected:
    void *m_user_data; ///< User data for the SymbolFile parser to store information into.
    lldb::LanguageType m_language; ///< The programming language enumeration value.
    Flags m_flags; ///< Compile unit flags that help with partial parsing.
    std::vector<lldb::FunctionSP> m_functions; ///< The sparsely populated list of shared pointers to functions
                                         ///< that gets populated as functions get partially parsed.
    FileSpecList m_support_files; ///< Files associated with this compile unit's line table and declarations.
    std::auto_ptr<LineTable> m_line_table_ap; ///< Line table that will get parsed on demand.
    lldb::VariableListSP m_variables; ///< Global and static variable list that will get parsed on demand.

private:
    enum
    {
        flagsParsedAllFunctions = (1u << 0), ///< Have we already parsed all our functions
        flagsParsedVariables    = (1u << 1), ///< Have we already parsed globals and statics?
        flagsParsedSupportFiles = (1u << 2), ///< Have we already parsed the support files for this compile unit?
        flagsParsedLineTable    = (1u << 3),  ///< Have we parsed the line table already?
        flagsParsedLanguage     = (1u << 4)   ///< Have we parsed the line table already?
    };

    DISALLOW_COPY_AND_ASSIGN (CompileUnit);
};

} // namespace lldb_private

#endif  // liblldb_CompUnit_h_
