#define MAL_STRIP_LOG_DEBUG //debug severity levels and below are stripped. Debug is the lowest severity, so just the entries with the debug severity are stripped in this case.

#include <cassert>
#include <mal_log/mal_log.hpp>
#include <mal_log/frontend.hpp> //UNCOMMENT IF YOU DON'T WANT TO COMPILE THE LIB SEPARATELY, COMMENT IF YOU DO
#include <mal_log/util/stack_ostream.hpp>

//------------------------------------------------------------------------------
inline mal::frontend& get_mal_logger_instance()
{
    static mal::frontend fe;
    return fe;
}
//------------------------------------------------------------------------------
void general_features()
{
    using namespace mal;
    mal::frontend& fe                 = get_mal_logger_instance();
    if (!fe.is_constructed()) {
        return; //new failed on static initializator
    }
    auto be_cfg                       = fe.get_backend_cfg();
    be_cfg.file.name_prefix           = "test-data.";
    be_cfg.file.name_suffix           = ".log.txt";
#ifndef MAL_WINDOWS
    be_cfg.file.out_folder            = "./log_out/";                             //this folder has to exist before running
#else
    be_cfg.file.out_folder            = ".\\log_out\\";                           //this folder has to exist before running
#endif
    be_cfg.file.aprox_size            = 512 * 1024;
    be_cfg.file.rotation.file_count   = 0;

    be_cfg.display.show_severity      = true;
    be_cfg.display.show_timestamp     = true;

    be_cfg.alloc.fixed_entry_size     = 32;
    be_cfg.alloc.fixed_block_size     = be_cfg.alloc.fixed_entry_size * 16;
    be_cfg.alloc.use_heap_if_required = true;

    if (fe.init_backend (be_cfg) != frontend::init_ok) { return; }

    fe.set_file_severity (sev::notice);
    fe.set_console_severity (sev::notice);
    int i = 0;

    log_error ("message {}, bool = {}", i, true); ++i;

    log_error ("message {}, u8  = {}", i, (u8) 8); ++i;
    log_error ("message {}, u16 = {}", i, (u16) 16); ++i;
    log_error ("message {}, u32 = {}", i, (u32) 32); ++i;
    log_error ("message {}, u64 = {}", i, (u64) 64); ++i;
    log_error ("message {}, i8  = {}", i, (i8) -8); ++i;
    log_error ("message {}, i16 = {}", i, (i16) -16); ++i;
    log_error ("message {}, i32 = {}", i, (i32) -32); ++i;
    log_error ("message {}, i64 = {}", i, (i64) -64); ++i;

    log_error ("message {}, hex u8  = {x}", i, (u8) 8); ++i;
    log_error ("message {}, hex u16 = {x}", i, (u16) 16); ++i;
    log_error ("message {}, hex u32 = {x}", i, (u32) 32); ++i;
    log_error ("message {}, hex u64 = {x}", i, (u64) 64); ++i;
    log_error ("message {}, hex i8  = {x}", i, (i8) -8); ++i;
    log_error ("message {}, hex i16 = {x}", i, (i16) -16); ++i;
    log_error ("message {}, hex i32 = {x}", i, (i32) -32); ++i;
    log_error ("message {}, hex i64 = {x}", i, (i64) -64); ++i;

    log_error ("message {}, wide u8  = {w}", i, (u8) 8); ++i;
    log_error ("message {}, wide u16 = {w}", i, (u16) 16); ++i;
    log_error ("message {}, wide u32 = {w}", i, (u32) 32); ++i;
    log_error ("message {}, wide u64 = {w}", i, (u64) 64); ++i;
    log_error ("message {}, wide i8  = {w}", i, (i8) 8); ++i;
    log_error ("message {}, wide i8  = {w}", i, (i8) -8); ++i;
    log_error ("message {}, wide i16 = {w}", i, (i16) 16); ++i;
    log_error ("message {}, wide i16 = {w}", i, (i16) -16); ++i;
    log_error ("message {}, wide i32 = {w}", i, (i32) 32); ++i;
    log_error ("message {}, wide i32 = {w}", i, (i32) -32); ++i;
    log_error ("message {}, wide i64 = {w}", i, (i64) 64); ++i;
    log_error ("message {}, wide i64 = {w}", i, (i64) -64); ++i;

    log_error ("message {}, double = {}", i, 12.1234567890123456789); ++i;
    log_error ("message {}, float = {}", i, 12.1234567890123456789f); ++i;

    log_error ("message {}, double sci = {s}", i, 12.1234567890123456789); ++i;
    log_error ("message {}, float sci = {s}", i, 12.1234567890123456789f); ++i;

    log_error ("message {}, double hex = {x}", i, 12.1234567890123456789); ++i;   //I added hex floats because representing small doubles at full precision can take hundreds of characters
    log_error ("message {}, float hex = {x}", i, 12.1234567890123456789f); ++i;

    log_error ("message {}, bool = {}", i, true); ++i;

    log_error ("message {}, ptr = {}", i, ptr (&fe)); ++i;

    const char* a_decayed_literal = "a compile time literal";

    log_error ("message {}, c_str = {}", i, lit (a_decayed_literal)); ++i;

    std::string string = "this is deep copied";
    log_error ("message {}, raw = {}", i, deep_copy (string)); ++i;

    u8 deep_copied_bytes[] = {
        0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab,
        0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    };
    log_error(
        "message {}, use with care: deeply copied bytes = {}",
        i,
        bytes (deep_copied_bytes, sizeof deep_copied_bytes)
        );
    ++i;
    log_error(
        "message {}, 1={}, 2={}, 3={}, 4={}, 5={x}",
        i,
        true,
        -1231,
        432.12f,
        lit ("param4"),
        (u64) -1
        );
    ++i;
#if !defined (MAL_COMPILE_TIME_FMT_CHECK) && defined (NDEBUG)
    log_error(
        "message {}, this could be corrected at compile time if all "
        "targeted compilers supported constexpr, now a runtime error "
        "here is the best I can do. An error will appear here {} and"
         " here too {}",
        i
        );
    ++i;
    log_error(
        "message {}, this could be corrected at compile time if all "
        "targeted compilers supported constexpr, now a runtime error "
        "here is the best I can do ",
        i,
        false
        );
    ++i;
    log_error(
        "message {}, this could be corrected at compile time if all "
        "targeted compilers supported constexpr, this placeholder has"
        " an invalid modifier that will be ignored {J}",
        i,
        32
        );
    ++i;
#endif //MAL_COMPILE_TIME_FMT_CHECK
    log_error ("message {}, this isn't interpreted as a placeholder {ww}", i);
    log_notice_if (true, "you should see this conditional entry...");
    log_notice_if (false, "...but not this one");

    for (unsigned j = 0; j < 15; ++j)  {
        log_every (4, log_notice ("every 4, count = {}", j));
    }
    bool called           = false;
    auto side_effect_expr = [&]()->bool { called = true; return called; };

    log_trace(
        "you shouldn't see this, this entry is below logged severity, as usual "
        "with most data loggers parameters are lazy evaluated, so be careful "
        "that no passed expression has side effects. {}",
        side_effect_expr()
        );
    assert (!called);

    fe.set_file_severity (sev::debug);
    fe.set_console_severity (sev::debug);
    log_trace(
        "now this is above the logged severity, you should see this, the "
        "function with side effects is called now: {}",
        side_effect_expr()
        );
    assert (called);

    log_debug(
        "you shouldn't see this, this entry is stripped at compile time"
        );

    log_error(
        log_fileline "this one should have the file and line prepended"
        );

    log_critical_sync(
        "this one blocks this thread until the log message is dequeued by the "
        "file writer, this call has a lot of overhead and scales poorly, so if "
        "you use the \"_sync\" calls often consider switching to a synchronous "
        "data logger."
        );

    stack_ostream<32> ostr;
    log_error(
        "I claim no support for ostreams because this is a very last resort "
        "hack to be able to print classes that don't expose members but are "
        "ostreamable. It still is stack based and lazy evaluated. Use only "
        "through the ugly macro like is done here. {}",
        ostr_deep_copy (ostr, "ostream hack: " << 1 << " " << 2 << " " << 3)
        );
    assert (!ostr.fail() && "this one wasn't truncated");

    ostr.reuse();
    log_error(
        "Now the stream will be truncated, you can check the fail bit. "
        "stream = {}",
        ostr_deep_copy (ostr, "1234567890123456789012345678901234567890")
        );
    assert (ostr.fail());
}
//------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
    general_features();
    return 0;
}
//------------------------------------------------------------------------------
