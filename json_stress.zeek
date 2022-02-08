redef LogAscii::use_json = T;

module JSONStress;

export {
    redef enum Log::ID += { LOG };
    type Info: record {
        data:   string  &log;
    };
}

event zeek_init() &priority=5
{
    Log::create_stream(JSONStress::LOG, [$columns=Info, $path="json_stress"]);
}

event log_all_data(f: fa_file, data: string)
{
    Log::write(JSONStress::LOG, [$data=data]);
}

event file_over_new_connection(f: fa_file, c: connection, is_orig: bool)
{
    Files::add_analyzer(f, Files::ANALYZER_DATA_EVENT, [$stream_event=log_all_data]);
}
