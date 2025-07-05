# "-i",
# "\\Device\\NPF_{3AB12F23-E38E-4B06-9814-D3F9E7007B0D}"

event zeek_init()
{
print Pcap::findalldevs();
}