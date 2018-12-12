/* automatically generated by rust-bindgen */
#![allow(dead_code)]
pub const PKT_RX_VLAN: u32 = 1;
pub const PKT_RX_RSS_HASH: u32 = 2;
pub const PKT_RX_FDIR: u32 = 4;
pub const PKT_RX_L4_CKSUM_BAD: u32 = 8;
pub const PKT_RX_IP_CKSUM_BAD: u32 = 16;
pub const PKT_RX_EIP_CKSUM_BAD: u32 = 32;
pub const PKT_RX_VLAN_STRIPPED: u32 = 64;
pub const PKT_RX_IP_CKSUM_MASK: u32 = 144;
pub const PKT_RX_IP_CKSUM_UNKNOWN: u32 = 0;
pub const PKT_RX_IP_CKSUM_GOOD: u32 = 128;
pub const PKT_RX_IP_CKSUM_NONE: u32 = 144;
pub const PKT_RX_L4_CKSUM_MASK: u32 = 264;
pub const PKT_RX_L4_CKSUM_UNKNOWN: u32 = 0;
pub const PKT_RX_L4_CKSUM_GOOD: u32 = 256;
pub const PKT_RX_L4_CKSUM_NONE: u32 = 264;
pub const PKT_RX_IEEE1588_PTP: u32 = 512;
pub const PKT_RX_IEEE1588_TMST: u32 = 1024;
pub const PKT_RX_FDIR_ID: u32 = 8192;
pub const PKT_RX_FDIR_FLX: u32 = 16384;
pub const PKT_RX_QINQ_STRIPPED: u32 = 32768;
pub const PKT_RX_LRO: u32 = 65536;
pub const PKT_RX_TIMESTAMP: u32 = 131072;
pub const PKT_RX_SEC_OFFLOAD: u32 = 262144;
pub const PKT_RX_SEC_OFFLOAD_FAILED: u32 = 524288;
pub const PKT_RX_QINQ: u32 = 1048576;
pub const PKT_TX_UDP_SEG: u64 = 4398046511104;
pub const PKT_TX_SEC_OFFLOAD: u64 = 8796093022208;
pub const PKT_TX_MACSEC: u64 = 17592186044416;
pub const PKT_TX_TUNNEL_VXLAN: u64 = 35184372088832;
pub const PKT_TX_TUNNEL_GRE: u64 = 70368744177664;
pub const PKT_TX_TUNNEL_IPIP: u64 = 105553116266496;
pub const PKT_TX_TUNNEL_GENEVE: u64 = 140737488355328;
pub const PKT_TX_TUNNEL_MPLSINUDP: u64 = 175921860444160;
pub const PKT_TX_TUNNEL_MASK: u64 = 527765581332480;
pub const PKT_TX_QINQ: u64 = 562949953421312;
pub const PKT_TX_QINQ_PKT: u64 = 562949953421312;
pub const PKT_TX_TCP_SEG: u64 = 1125899906842624;
pub const PKT_TX_IEEE1588_TMST: u64 = 2251799813685248;
pub const PKT_TX_L4_NO_CKSUM: u32 = 0;
pub const PKT_TX_TCP_CKSUM: u64 = 4503599627370496;
pub const PKT_TX_SCTP_CKSUM: u64 = 9007199254740992;
pub const PKT_TX_UDP_CKSUM: u64 = 13510798882111488;
pub const PKT_TX_L4_MASK: u64 = 13510798882111488;
pub const PKT_TX_IP_CKSUM: u64 = 18014398509481984;
pub const PKT_TX_IPV4: u64 = 36028797018963968;
pub const PKT_TX_IPV6: u64 = 72057594037927936;
pub const PKT_TX_VLAN: u64 = 144115188075855872;
pub const PKT_TX_VLAN_PKT: u64 = 144115188075855872;
pub const PKT_TX_OUTER_IP_CKSUM: u64 = 288230376151711744;
pub const PKT_TX_OUTER_IPV4: u64 = 576460752303423488;
pub const PKT_TX_OUTER_IPV6: u64 = 1152921504606846976;
pub const PKT_TX_OFFLOAD_MASK: u64 = 468365565153509376;
pub const __RESERVED: u64 = 2305843009213693952;
pub const IND_ATTACHED_MBUF: u64 = 4611686018427387904;
pub const CTRL_MBUF_FLAG: i64 = - 9223372036854775808;