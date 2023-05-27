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
    // printf("ip_in\n");
    // TO-DO
    // 判断IP头部大小
    if (buf->len < IP_HDR_LEN) {
        return;
    }
    // 获取数据
    ip_hdr_t *hrd = (ip_hdr_t *)buf->data;
    // printf("00\n");
    // printf("%d,%d,%d", hrd->version, swap16(hrd->total_len16), buf->len);
    // 报头检测
    if (hrd->version != IP_VERSION_4 || swap16(hrd->total_len16) > buf->len){
        return;
    }
    // 保存首部校验和并置零
    uint16_t checksum16_save = hrd->hdr_checksum16;
    hrd->hdr_checksum16 = 0;
    // printf("11\n");
    if (checksum16_save != checksum16(buf->data, IP_HDR_LEN))
    {
        return;
    }
    // printf("22\n");
    // 恢复校验和
    hrd->hdr_checksum16 = checksum16_save;
    // 判断ip地址是否为本机ip地址
    if (memcmp(hrd->dst_ip, net_if_ip, NET_IP_LEN)!=0) {
        return;
    }
    // printf("33\n");
    // 去除填充字段
    if (buf->len > hrd->total_len16) {
        buf_remove_padding(buf, (buf->len) - (hrd->total_len16));
    }
    
    // 向上层传递
    int protocol = hrd->protocol;
    // printf("ip protocol %d\n", protocol);
    if (protocol == NET_PROTOCOL_ICMP || protocol == NET_PROTOCOL_UDP || protocol == NET_PROTOCOL_TCP) {    //  || protocol == NET_PROTOCOL_TCP
        // 去除ip报头
        uint8_t src_ip_[NET_IP_LEN];
        memcpy(src_ip_, hrd->src_ip, NET_IP_LEN);
        buf_remove_header(buf, IP_HDR_LEN);
        net_in(buf, protocol, src_ip_);
    }
    else {
        icmp_unreachable(buf, hrd->src_ip, ICMP_CODE_PROTOCOL_UNREACH);
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
    // printf("ip_fragment_out\n");
    // 添加包头
    buf_add_header(buf, IP_HDR_LEN);
    ip_hdr_t *hrd = (ip_hdr_t *)buf->data;
    // printf("IP_HDR_LEN :%d\n", IP_HDR_LEN);
    // printf("ip len:%d\n", buf->len);
    // 填写包头
    hrd->hdr_len = IP_HDR_LEN / IP_HDR_LEN_PER_BYTE;
    hrd->version = IP_VERSION_4;
    hrd->tos = 0;
    hrd->total_len16 = swap16(buf->len);
    hrd->id16 = swap16(id);
    hrd->flags_fragment16 = swap16(mf == 1 ? IP_MORE_FRAGMENT | offset : offset);
    hrd->ttl = 64;
    hrd->protocol = protocol;
    hrd->hdr_checksum16 = 0;
    memcpy(hrd->src_ip, net_if_ip, NET_IP_LEN);
    memcpy(hrd->dst_ip, ip, NET_IP_LEN);
    hrd->hdr_checksum16 = (checksum16(buf->data, IP_HDR_LEN));
    // printf("checkout sum %x\n", hrd->hdr_checksum16);
    // for (int i = 0; i < buf->len; i++)
    // {
    //     printf("%d\n", buf->data[i]);
    // }

    // 调用arp_out发送
    arp_out(buf, ip);
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
    // printf("ip__out\n");
    // printf("ip data len:%d\n", buf->len);
    uint32_t tmp_id = id++;
    if (buf->len > (MTU - IP_HDR_LEN))
    {
        // 分片可封装数据大小
        int d = (MTU - IP_HDR_LEN) / 8;
        d *= 8;
        // 片数
        int n = (buf->len - IP_HDR_LEN) / d;
        if ((buf->len - IP_HDR_LEN) % d != 0)
            n++;
        // 发送各片(还未添加报头)
        for (int i = 0; i < n-1; i++)
        {
            // printf("offset8:%d\n", i * d);
            buf_init(&txbuf, MTU - IP_HDR_LEN);
            memcpy(txbuf.data, (buf->data + d * i), d);
            ip_fragment_out(&txbuf, ip, protocol, tmp_id, i * d / 8, 1);
        }
        // 最后一片
        // printf("offset8:%d\n", (n-1) * d);
        buf_init(&txbuf, buf->len - d*(n-1));
        memcpy(txbuf.data, (buf->data + d * (n-1)), buf->len - d*(n-1));
        ip_fragment_out(&txbuf, ip, protocol, tmp_id, (n-1) * d / 8, 0);
    }
    else {
        ip_fragment_out(buf, ip, protocol, tmp_id, 0, 0);
    }
}

/**
 * @brief 初始化ip协议
 * 
 */
void ip_init()
{
    net_add_protocol(NET_PROTOCOL_IP, ip_in);
}