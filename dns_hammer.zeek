module DNS;

event spew_dns ()
{
    local info: Info;
    local id: conn_id;
    id$orig_h = 1.2.3.4;
    id$orig_p = 23121/udp;
    id$resp_h = 5.6.7.8;
    id$resp_p = 53/udp;
    info$ts       = network_time();
    info$uid = "CzCai71J2P10svwng2";
    info$id=id;
    info$proto=udp;
    info$query="www.google.com";
    info$qclass=1;
    info$qclass_name="C_INTERNET";
    info$qtype=1;
    info$qtype_name="A";
    info$rcode=0;
    info$rcode_name="NOERROR";
    info$AA=F;
    info$TC=F;
    info$RD=T;
    info$RA=T;
    info$Z=0;
    info$answers=["74.125.225.116", "74.125.225.112", "74.125.225.115", "74.125.225.114", "74.125.225.113"];
    info$TTLs=[84.0 secs, 84.0 secs, 64.0 secs, 84.0 secs, 84.0 secs];
    info$rejected=F;
    info$total_answers=5;
    info$total_replies=13;
    local x=0;
    while (++x < 1000)
         Log::write(DNS::LOG, info);
    schedule 75msec { spew_dns() };
}

event zeek_init()
{
    if ( Cluster::local_node_type() == Cluster::WORKER )
    {
        schedule 5sec { spew_dns() };
    }
}