#include "ethernet.h"
#include "utils.h"
#include "driver.h"
#include "arp.h"
#include "ip.h"
/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf)
{
    // TO-DO
    // 判断mac头部大小
    if (buf->len <= MAC_HEADER_LEN) {
        return;
    }
    // 获取包头
    ether_hdr_t *hrd = (ether_hdr_t *)buf->data;
    uint8_t src[NET_MAC_LEN];
    memcpy(src, hrd->src, NET_MAC_LEN);
    net_protocol_t protocol = swap16(hrd->protocol16);

    // 移除eth包头
    buf_remove_header(buf, sizeof(ether_hdr_t));
    // 向上层传递数据包
    net_in(buf, protocol, src);

}
/**
 * @brief 处理一个要发送的数据包
 * 
 * @param buf 要处理的数据包
 * @param mac 目标MAC地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol)
{
    // TO-DO
    // 数据包太小则填充
    if (buf->len <= ETHERNET_MIN_TRANSPORT_UNIT) {
        buf_add_padding(buf, ETHERNET_MIN_TRANSPORT_UNIT - buf->len);
    }
    // 填充包头
    buf_add_header(buf, sizeof(ether_hdr_t));
    ether_hdr_t *hrd = (ether_hdr_t *)buf->data;
    // 设置包头
    uint8_t local_mac[NET_MAC_LEN] = NET_IF_MAC;
    memcpy(hrd->src, local_mac, NET_MAC_LEN);
    memcpy(hrd->dst, mac, NET_MAC_LEN);
    hrd->protocol16 = swap16(protocol);
    // 发送函数
    driver_send(buf);
}
/**
 * @brief 初始化以太网协议
 * 
 */
void ethernet_init()
{
    buf_init(&rxbuf, ETHERNET_MAX_TRANSPORT_UNIT + sizeof(ether_hdr_t));
}

/**
 * @brief 一次以太网轮询
 * 
 */
void ethernet_poll()
{
    if (driver_recv(&rxbuf) > 0)
        ethernet_in(&rxbuf);
}
