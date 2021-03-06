//===-- Watchpoint.h --------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_Watchpoint_h_
#define liblldb_Watchpoint_h_

// C Includes

// C++ Includes
#include <list>
#include <string>

// Other libraries and framework includes

// Project includes
#include "lldb/lldb-private.h"
#include "lldb/Target/Target.h"
#include "lldb/Core/UserID.h"
#include "lldb/Breakpoint/WatchpointOptions.h"
#include "lldb/Breakpoint/StoppointLocation.h"

namespace lldb_private {

class Watchpoint :
    public STD_ENABLE_SHARED_FROM_THIS(Watchpoint),
    public StoppointLocation
{
public:

    class WatchpointEventData :
        public EventData
    {
    public:

        static const ConstString &
        GetFlavorString ();

        virtual const ConstString &
        GetFlavor () const;

        WatchpointEventData (lldb::WatchpointEventType sub_type,
                             const lldb::WatchpointSP &new_watchpoint_sp);

        virtual
        ~WatchpointEventData();

        lldb::WatchpointEventType
        GetWatchpointEventType () const;

        lldb::WatchpointSP &
        GetWatchpoint ();
        
        virtual void
        Dump (Stream *s) const;

        static lldb::WatchpointEventType
        GetWatchpointEventTypeFromEvent (const lldb::EventSP &event_sp);

        static lldb::WatchpointSP
        GetWatchpointFromEvent (const lldb::EventSP &event_sp);

        static const WatchpointEventData *
        GetEventDataFromEvent (const Event *event_sp);

    private:

        lldb::WatchpointEventType m_watchpoint_event;
        lldb::WatchpointSP m_new_watchpoint_sp;

        DISALLOW_COPY_AND_ASSIGN (WatchpointEventData);
    };

    Watchpoint (Target& target, lldb::addr_t addr, size_t size, const ClangASTType *type, bool hardware = true);
    ~Watchpoint ();

    void
    IncrementFalseAlarmsAndReviseHitCount();

    bool
    IsEnabled () const;

    void
    SetEnabled (bool enabled, bool notify = true);

    virtual bool
    IsHardware () const;

    virtual bool
    ShouldStop (StoppointCallbackContext *context);

    bool        WatchpointRead () const;
    bool        WatchpointWrite () const;
    uint32_t    GetIgnoreCount () const;
    void        SetIgnoreCount (uint32_t n);
    void        SetWatchpointType (uint32_t type, bool notify = true);
    void        SetDeclInfo (const std::string &str);
    std::string GetWatchSpec();
    void        SetWatchSpec (const std::string &str);

    // Snapshot management interface.
    bool        IsWatchVariable() const;
    void        SetWatchVariable(bool val);
    bool        CaptureWatchedValue (const ExecutionContext &exe_ctx);

    void        GetDescription (Stream *s, lldb::DescriptionLevel level);
    void        Dump (Stream *s) const;
    void        DumpSnapshots (Stream *s, const char * prefix = NULL) const;
    void        DumpWithLevel (Stream *s, lldb::DescriptionLevel description_level) const;
    Target      &GetTarget() { return m_target; }
    const Error &GetError() { return m_error; }

    //------------------------------------------------------------------
    /// Returns the WatchpointOptions structure set for this watchpoint.
    ///
    /// @return
    ///     A pointer to this watchpoint's WatchpointOptions.
    //------------------------------------------------------------------
    WatchpointOptions *
    GetOptions () { return &m_options; }

    //------------------------------------------------------------------
    /// Set the callback action invoked when the watchpoint is hit.  
    /// 
    /// @param[in] callback
    ///    The method that will get called when the watchpoint is hit.
    /// @param[in] callback_baton
    ///    A void * pointer that will get passed back to the callback function.
    /// @param[in] is_synchronous
    ///    If \b true the callback will be run on the private event thread
    ///    before the stop event gets reported.  If false, the callback will get
    ///    handled on the public event thead after the stop has been posted.
    ///
    /// @return
    ///    \b true if the process should stop when you hit the watchpoint.
    ///    \b false if it should continue.
    //------------------------------------------------------------------
    void
    SetCallback (WatchpointHitCallback callback, 
                 void *callback_baton,
                 bool is_synchronous = false);

    void
    SetCallback (WatchpointHitCallback callback, 
                 const lldb::BatonSP &callback_baton_sp,
                 bool is_synchronous = false);

    void        ClearCallback();

    //------------------------------------------------------------------
    /// Invoke the callback action when the watchpoint is hit.
    ///
    /// @param[in] context
    ///     Described the watchpoint event.
    ///
    /// @return
    ///     \b true if the target should stop at this watchpoint and \b false not.
    //------------------------------------------------------------------
    bool
    InvokeCallback (StoppointCallbackContext *context);

    //------------------------------------------------------------------
    // Condition
    //------------------------------------------------------------------
    //------------------------------------------------------------------
    /// Set the watchpoint's condition.
    ///
    /// @param[in] condition
    ///    The condition expression to evaluate when the watchpoint is hit.
    ///    Pass in NULL to clear the condition.
    //------------------------------------------------------------------
    void SetCondition (const char *condition);
    
    //------------------------------------------------------------------
    /// Return a pointer to the text of the condition expression.
    ///
    /// @return
    ///    A pointer to the condition expression text, or NULL if no
    //     condition has been set.
    //------------------------------------------------------------------
    const char *GetConditionText () const;

    void
    TurnOnEphemeralMode();

    void
    TurnOffEphemeralMode();

    bool
    IsDisabledDuringEphemeralMode();
    
    const ClangASTType &
    GetClangASTType()
    {
        return m_type;
    }


private:
    friend class Target;
    friend class WatchpointList;

    void        ResetHitCount() { m_hit_count = 0; }

    Target      &m_target;
    bool        m_enabled;             // Is this watchpoint enabled
    bool        m_is_hardware;         // Is this a hardware watchpoint
    bool        m_is_watch_variable;   // True if set via 'watchpoint set variable'.
    bool        m_is_ephemeral;        // True if the watchpoint is in the ephemeral mode, meaning that it is
                                       // undergoing a pair of temporary disable/enable actions to avoid recursively
                                       // triggering further watchpoint events.
    uint32_t    m_disabled_count;      // Keep track of the count that the watchpoint is disabled while in ephemeral mode.
                                       // At the end of the ephemeral mode when the watchpoint is to be enabled agian,
                                       // we check the count, if it is more than 1, it means the user-supplied actions
                                       // actually want the watchpoint to be disabled!
    uint32_t    m_watch_read:1,        // 1 if we stop when the watched data is read from
                m_watch_write:1,       // 1 if we stop when the watched data is written to
                m_watch_was_read:1,    // Set to 1 when watchpoint is hit for a read access
                m_watch_was_written:1; // Set to 1 when watchpoint is hit for a write access
    uint32_t    m_ignore_count;        // Number of times to ignore this watchpoint
    uint32_t    m_false_alarms;        // Number of false alarms.
    std::string m_decl_str;            // Declaration information, if any.
    std::string m_watch_spec_str;      // Spec for the watchpoint.
    lldb::ValueObjectSP m_old_value_sp;
    lldb::ValueObjectSP m_new_value_sp;
    ClangASTType m_type;
    Error       m_error;               // An error object describing errors associated with this watchpoint.
    WatchpointOptions m_options;       // Settable watchpoint options, which is a delegate to handle
                                       // the callback machinery.
    bool        m_being_created;

    std::auto_ptr<ClangUserExpression> m_condition_ap;  // The condition to test.

    void SetID(lldb::watch_id_t id) { m_loc_id = id; }
    
    void
    SendWatchpointChangedEvent (lldb::WatchpointEventType eventKind);

    void
    SendWatchpointChangedEvent (WatchpointEventData *data);

    DISALLOW_COPY_AND_ASSIGN (Watchpoint);
};

} // namespace lldb_private

#endif  // liblldb_Watchpoint_h_
