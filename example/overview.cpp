#define UFO_STRIP_LOG_SEVERITY 0

#include <ufo_log/frontend_def.hpp>                                             //compiled in place, but it could be in a separate library
#include <ufo_log/ufo_log.hpp>
//------------------------------------------------------------------------------
inline ufo::frontend& get_ufo_logger_instance()
{
    static ufo::frontend fe;
    return fe;
}
//------------------------------------------------------------------------------
void general_features()
{
    using namespace ufo;
    ufo::frontend& fe               = get_ufo_logger_instance();
    auto be_cfg                     = fe.get_backend_cfg();
    be_cfg.file.name_prefix         = "test-data.";
    be_cfg.file.name_suffix         = ".log.txt";
    be_cfg.file.out_folder          = "./log_out/";                             //this folder has to exist before running
    be_cfg.file.aprox_size          = 512 * 1024;
    be_cfg.file.rotation.file_count = 0;

    be_cfg.display.show_severity    = true;
    be_cfg.display.show_timestamp   = true;

    if (fe.init_backend (be_cfg) != frontend::init_ok) { return; }

    fe.set_file_severity (sev::notice);
    fe.set_console_severity (sev::notice);
    int i = 0;

    std::string string = "this is deep copied";
    log_error ("message {}, raw = {}", ++i, deep_copy (string));

    log_error ("message {}, bool = {}", ++i, true);

    log_error ("message {}, u8  = {}", ++i, (u8) 8);
    log_error ("message {}, u16 = {}", ++i, (u16) 16);
    log_error ("message {}, u32 = {}", ++i, (u32) 32);
    log_error ("message {}, u64 = {}", ++i, (u64) 64);
    log_error ("message {}, i8  = {}", ++i, (i8) -8);
    log_error ("message {}, i16 = {}", ++i, (i16) -16);
    log_error ("message {}, i32 = {}", ++i, (i32) -32);
    log_error ("message {}, i64 = {}", ++i, (i64) -64);

    log_error ("message {}, hex u8  = {x}", ++i, (u8) 8);
    log_error ("message {}, hex u16 = {x}", ++i, (u16) 16);
    log_error ("message {}, hex u32 = {x}", ++i, (u32) 32);
    log_error ("message {}, hex u64 = {x}", ++i, (u64) 64);
    log_error ("message {}, hex i8  = {x}", ++i, (i8) -8);
    log_error ("message {}, hex i16 = {x}", ++i, (i16) -16);
    log_error ("message {}, hex i32 = {x}", ++i, (i32) -32);
    log_error ("message {}, hex i64 = {x}", ++i, (i64) -64);

    log_error ("message {}, wide u8  = {w}", ++i, (u8) 8);
    log_error ("message {}, wide u16 = {w}", ++i, (u16) 16);
    log_error ("message {}, wide u32 = {w}", ++i, (u32) 32);
    log_error ("message {}, wide u64 = {w}", ++i, (u64) 64);
    log_error ("message {}, wide i8  = {w}", ++i, (i8) 8);
    log_error ("message {}, wide i8  = {w}", ++i, (i8) -8);
    log_error ("message {}, wide i16 = {w}", ++i, (i16) 16);
    log_error ("message {}, wide i16 = {w}", ++i, (i16) -16);
    log_error ("message {}, wide i32 = {w}", ++i, (i32) 32);
    log_error ("message {}, wide i32 = {w}", ++i, (i32) -32);
    log_error ("message {}, wide i64 = {w}", ++i, (i64) 64);
    log_error ("message {}, wide i64 = {w}", ++i, (i64) -64);

    log_error ("message {}, double = {}", ++i, 12.1234567890123456789);
    log_error ("message {}, float = {}", ++i, 12.1234567890123456789f);

    log_error ("message {}, double sci = {s}", ++i, 12.1234567890123456789);
    log_error ("message {}, float sci = {s}", ++i, 12.1234567890123456789f);

    log_error ("message {}, double hex = {x}", ++i, 12.1234567890123456789);    //I added hex floats because representing small doubles at full precision can take hundreds of characters
    log_error ("message {}, float hex = {x}", ++i, 12.1234567890123456789f);

    log_error ("message {}, bool = {}", ++i, true);

#warning "todo"
//    log_error ("message {}, ptr = {}", ++i, ptr (&fe));
    log_error(
        "message {}, c_str = {}",
        ++i,
        "a literal decayed to const char* that has whole-program lifetime"
        );

    u8 deep_copied_bytes[] =
    {
        0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab,
        0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    };

    log_error(
        "message {}, use with care: deeply copied bytes = {}",
        ++i,
        bytes (deep_copied_bytes, sizeof deep_copied_bytes)
        );
    log_error(
        "message {}, 1={}, 2={}, 3={}, 4={}, 5={x}",
        ++i,
        true,
        -1231,
        432.12f,
        "param4",
        (u64) -1
        );
#ifndef UFO_COMPILE_TIME_FMT_CHECK
    log_error(
        "message {}, this could be corrected at compile time if all "
        "targeted compilers supported constexpr, now a runtime error "
        "here is the best I can do. An error will appear here {} and"
         " here too {}",
        ++i
        );
    log_error(
        "message {}, this could be corrected at compile time if all "
        "targeted compilers supported constexpr, now a runtime error "
        "here is the best I can do ",
        ++i,
        false
        );
    log_error(
        "message {}, this could be corrected at compile time if all "
        "targeted compilers supported constexpr, this placeholder has"
        " an invalid modifier that will be ignored {J}",
        ++i,
        32
        );
#endif //UFO_COMPILE_TIME_FMT_CHECK
    log_error ("message {}, this isn't interpreted as a placeholder {ww}", ++i);
    log_if (true, log_notice ("you should see this conditional entry..."));
    log_if (false, log_notice ("...but not this one"));

    for (unsigned i = 0; i < 15; ++i)
    {
        log_every (4, log_notice ("every 4, count = {}", i));
    }

    log_trace(
        "you shouldn't see this, this entry is below logged severity"
        );

    fe.set_file_severity (sev::debug);
    fe.set_console_severity (sev::debug);

    log_trace(
        "now this is above the logged severity, you should see this"
        );

    log_debug(
        "you shouldn't see this, this entry is stripped at compile time"
        );

    log_error(
        log_fileline "this one should have the file and line prepended"
        );
}
//------------------------------------------------------------------------------
int main (int argc, const char* argv[])
{
    general_features();
    return 0;
}
//------------------------------------------------------------------------------
