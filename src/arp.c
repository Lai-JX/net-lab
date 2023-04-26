#include <string.h>
#include <stdio.h>
#include "net.h"
#include "arp.h"
#include "ethernet.h"
/**
 * @brief 初始的arp包
 * 
 */
static const arp_pkt_t arp_init_pkt = {
    .hw_type16 = constswap16(ARP_HW_ETHER),
    .pro_type16 = constswap16(NET_PROTOCOL_IP),
    .hw_len = NET_MAC_LEN,
    .pro_len = NET_IP_LEN,
    // .opcode16 = constswap16(ARP_REQUEST),
    .sender_ip = NET_IF_IP,
    .sender_mac = NET_IF_MAC,
    .target_ip = NET_IF_IP,
    .target_mac = {0}};

/**
 * @brief arp地址转换表，<ip,mac>的容器
 * 
 */
map_t arp_table;

/**
 * @brief arp buffer，<ip,buf_t>的容器
 * 
 */
map_t arp_buf;

/**
 * @brief 打印一条arp表项
 * 
 * @param ip 表项的ip地址
 * @param mac 表项的mac地址
 * @param timestamp 表项的更新时间
 */
void arp_entry_print(void *ip, void *mac, time_t *timestamp)
{
    printf("%s | %s | %s\n", iptos(ip), mactos(mac), timetos(*timestamp));
}

/**
 * @brief 打印整个arp表
 * 
 */
void arp_print()
{
    printf("===ARP TABLE BEGIN===\n");
    map_foreach(&arp_table, arp_entry_print);
    printf("===ARP TABLE  END ===\n");
}

/**
 * @brief 发送一个arp请求 网卡启动时发出
 * 
 * @param target_ip 想要知道的目标的ip地址
 */
void arp_req(uint8_t *target_ip)
{
    // TO-DO
    // 初始化网卡发送缓冲区
    // 声明buf并封装arp  注：这里无需填充，因为ethernet_out发现数据包小于46时会进行填充
    buf_init(&txbuf, sizeof(arp_pkt_t));
    arp_pkt_t *arp_pkt = (arp_pkt_t *)(txbuf.data);
    memcpy(arp_pkt, &arp_init_pkt, sizeof(arp_pkt_t));
    memcpy(arp_pkt->target_ip, target_ip, NET_IP_LEN);
    arp_pkt->opcode16 = swap16(ARP_REQUEST);

    // 调用ethernet_out发送
    ethernet_out(&txbuf, ether_broadcast_mac, NET_PROTOCOL_ARP);
}

/**
 * @brief 发送一个arp响应
 * 
 * @param target_ip 目标ip地址
 * @param target_mac 目标mac地址
 */
void arp_resp(uint8_t *target_ip, uint8_t *target_mac)
{
    // TO-DO
    // 初始化网卡发送缓冲区
    // 声明buf并封装arp  注：这里无需填充，因为ethernet_out发现数据包小于46时会进行填充
    buf_init(&txbuf, sizeof(arp_pkt_t));
    arp_pkt_t *arp_pkt = (arp_pkt_t *)(txbuf.data);
    memcpy(arp_pkt, &arp_init_pkt, sizeof(arp_pkt_t));
    memcpy(arp_pkt->target_ip, target_ip, NET_IP_LEN);
    memcpy(arp_pkt->target_mac, target_mac, NET_MAC_LEN);
    arp_pkt->opcode16 = swap16(ARP_REPLY);

    // 调用ethernet_out发送
    ethernet_out(&txbuf, target_mac, NET_PROTOCOL_ARP);
}

/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 * @param src_mac 源mac地址
 */
void arp_in(buf_t *buf, uint8_t *src_mac)
{
    // TO-DO
    // 判断arp头部大小
    if (buf->len <= ARP_HEADER_LEN) {
        return;
    }
    // 获取数据
    arp_pkt_t *hrd = (arp_pkt_t *)buf->data;
    // 报头检测
    if (hrd->hw_type16 != arp_init_pkt.hw_type16 || hrd->pro_type16 != arp_init_pkt.pro_type16 ||
        hrd->hw_len != arp_init_pkt.hw_len || hrd->pro_len != arp_init_pkt.pro_len ||
        !(hrd->opcode16 == swap16(ARP_REQUEST) || hrd->opcode16 == swap16(ARP_REPLY))){
        return;
    }
    uint8_t ip[NET_IP_LEN];
    uint8_t mac[NET_MAC_LEN];
    memcpy(ip, hrd->sender_ip, NET_IP_LEN);
    memcpy(mac, hrd->sender_mac, NET_MAC_LEN);
    // uint8_t local_ip[NET_IP_LEN] = NET_IF_IP;
    // uint8_t local_mac[NET_MAC_LEN] = NET_IF_MAC;

    // 更新arp表
    map_set(&arp_table, ip, mac);
    // map_get查看是否有buf缓存
    buf_t *buf_ = map_get(&arp_buf, ip);
    if (buf_ == NULL)
    {
        if (hrd->opcode16 == swap16(ARP_REQUEST) && memcmp(hrd->target_ip, net_if_ip, NET_IP_LEN)==0)
        {
            arp_resp(ip, mac);
        }
    }
    else
    {
        ethernet_out(buf_, mac, NET_PROTOCOL_IP);
        map_delete(&arp_buf, ip);
    }
}

/**
 * @brief 处理一个要发送的数据包
 * 
 * @param buf 要处理的数据包
 * @param ip 目标ip地址
 * @param protocol 上层协议
 */
void arp_out(buf_t *buf, uint8_t *ip)
{
    // TO-DO
    uint8_t* mac = map_get(&arp_table, ip);
    if (mac != NULL){
        ethernet_out(buf, mac, NET_PROTOCOL_IP);
        return;
    }
    buf_t *buf_ = map_get(&arp_buf, ip);
    if (buf_ != NULL)
    {
        return;
    }

    // 更新ip对应的缓存数据包
    map_set(&arp_buf, ip, buf);
    arp_req(ip);
}

/**
 * @brief 初始化arp协议
 * 
 */
void arp_init()
{
    map_init(&arp_table, NET_IP_LEN, NET_MAC_LEN, 0, ARP_TIMEOUT_SEC, NULL);
    map_init(&arp_buf, NET_IP_LEN, sizeof(buf_t), 0, ARP_MIN_INTERVAL, buf_copy);
    net_add_protocol(NET_PROTOCOL_ARP, arp_in);
    arp_req(net_if_ip);
}