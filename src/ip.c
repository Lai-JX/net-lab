#include "net.h"
#include "ip.h"
#include "ethernet.h"
#include "arp.h"
#include "icmp.h"

/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 * @param src_mac 源mac地址
 */
void ip_in(buf_t *buf, uint8_t *src_mac)
{
    // TO-DO
    // 判断arp头部大小
    if (buf->len < IP_HDR_LEN) {
        return;
    }
    // 获取数据
    ip_hdr_t *hrd = (ip_hdr_t *)buf->data;
    // 报头检测
    if (hrd->version != IP_VERSION_4 || hrd->total_len16 > buf->len){
        return;
    }
    // 保存首部校验和并置零
    uint16_t checksum16_save = hrd->hdr_checksum16;
    hrd->hdr_checksum16 = 0;
    if (checksum16_save != checksum16(buf->data, IP_HDR_LEN)) {
        return;
    }
    // 恢复校验和
    hrd->hdr_checksum16 = checksum16_save;
    // 判断ip地址是否为本机ip地址
    if (memcmp(hrd->dst_ip, net_if_ip, NET_IP_LEN)!=0) {
        return;
    }
    // 去除填充字段
    if (buf->len > hrd->total_len16) {
        buf_remove_padding(buf, (buf->len) - (hrd->total_len16));
    }
    // 去除ip报头
    buf_remove_header(buf, IP_HDR_LEN);
    // 向上层传递
    int protocol = hrd->protocol;
    if (protocol == NET_PROTOCOL_ICMP || protocol == NET_PROTOCOL_UDP || protocol == NET_PROTOCOL_TCP) {
        net_in(buf, hrd->protocol, hrd->src_ip);
    }
    else {
        icmp_unreachable(buf, hrd->src_ip, ICMP_CODE_PORT_UNREACH);
    }
}

/**
 * @brief 处理一个要发送的ip分片
 * 
 * @param buf 要发送的分片
 * @param ip 目标ip地址
 * @param protocol 上层协议
 * @param id 数据包id
 * @param offset 分片offset，必须被8整除
 * @param mf 分片mf标志，是否有下一个分片
 */
void ip_fragment_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol, int id, uint16_t offset, int mf)
{
    // TO-DO
}

/**
 * @brief 处理一个要发送的ip数据包
 * 
 * @param buf 要处理的包
 * @param ip 目标ip地址
 * @param protocol 上层协议
 */
void ip_out(buf_t *buf, uint8_t *ip, net_protocol_t protocol)
{
    // TO-DO
}

/**
 * @brief 初始化ip协议
 * 
 */
void ip_init()
{
    net_add_protocol(NET_PROTOCOL_IP, ip_in);
}