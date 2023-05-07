#include "udp.h"
#include "ip.h"
#include "icmp.h"

/**
 * @brief udp处理程序表
 * 
 */
map_t udp_table;

/**
 * @brief udp伪校验和计算
 * 
 * @param buf 要计算的包
 * @param src_ip 源ip地址
 * @param dst_ip 目的ip地址
 * @return uint16_t 伪校验和
 */
static uint16_t udp_checksum(buf_t *buf, uint8_t *src_ip, uint8_t *dst_ip)
{
    // TO-DO
    // 增加伪头部
    buf_add_header(buf, sizeof(udp_peso_hdr_t));
    // 拷贝被覆盖的ip头部
    ip_hdr_t ip_hdr;
    memcpy(&ip_hdr, buf->data + sizeof(udp_peso_hdr_t) - sizeof(ip_hdr_t), sizeof(ip_hdr_t));
    // 填写伪头部
    udp_peso_hdr_t *hrd = (udp_peso_hdr_t *)buf->data;
    memcpy(hrd->dst_ip, dst_ip, NET_IP_LEN);
    memcpy(hrd->src_ip, src_ip, NET_IP_LEN);
    hrd->placeholder = 0;
    hrd->protocol = NET_PROTOCOL_UDP;
    hrd->total_len16 = swap16(buf->len - sizeof(udp_peso_hdr_t));       // 这个需要转！！！！
    int flag = 0;
    // 填充
    if (buf->len % 2) 
    {
        buf_add_padding(buf, 1);
        flag = 1;
    }
    // printf("checksum len:%d\n", buf->len);
    // for (int i = 0; i < buf->len; i++)
    // {
    //     printf("%x\t", buf->data[i]);
    // }

    // 计算校验和
    uint16_t checksum = checksum16(buf->data, buf->len);
    // 恢复ip头部
    memcpy(buf->data + sizeof(udp_hdr_t) - sizeof(ip_hdr_t), &ip_hdr, sizeof(ip_hdr_t));
    // 去掉伪头部和填充
    buf_remove_header(buf, sizeof(udp_peso_hdr_t));
    if (flag)
        buf_remove_padding(buf, 1);

    // 返回
    // printf("checksum:%x\n", checksum);
    return checksum;
}

/**
 * @brief 处理一个收到的udp数据包
 * 
 * @param buf 要处理的包
 * @param src_ip 源ip地址
 */
void udp_in(buf_t *buf, uint8_t *src_ip)
{
    // TO-DO
    // printf("udp_in\n");
    // printf("udp_in buf-len:%d\n",buf->len);
    // 判断udp头部大小
    if (buf->len < sizeof(udp_hdr_t)) {
        return;
    }
    // 获取数据
    udp_hdr_t *hrd = (udp_hdr_t *)buf->data;
    // 保存首部校验和并置零
    uint16_t checksum16_save = hrd->checksum16;
    // printf("00\n");
    hrd->checksum16 = 0;
    // printf("checksum_save:%x\n", checksum16_save);
    if (checksum16_save != udp_checksum(buf, src_ip, net_if_ip))
    {
        // printf("ret\n");
        return;
    }
    // printf("pppppppppppppppp\n");
    // 恢复校验和
    hrd->checksum16 = checksum16_save;

    uint16_t port = swap16(hrd->dst_port16);
    udp_handler_t *handler = map_get(&udp_table, &(port));
    // printf("11\n");
    // 是否有目的端口对应的处理函数
    if(handler)
    {
        // printf("22\n");
        buf_remove_header(buf, sizeof(udp_hdr_t));
        (*handler)(buf->data, buf->len, src_ip, port);
    }
    else
    {
        // printf("33\n");
        buf_add_header(buf, sizeof(ip_hdr_t));
        ip_hdr_t *hrd = (ip_hdr_t *)buf->data;

        // 填写包头
        hrd->hdr_len = IP_HDR_LEN / IP_HDR_LEN_PER_BYTE;
        hrd->version = IP_VERSION_4;
        hrd->tos = 0;
        hrd->total_len16 = swap16(buf->len);
        hrd->id16 = swap16(id++);
        hrd->flags_fragment16 = 0;
        hrd->ttl = 64;
        hrd->protocol = NET_PROTOCOL_UDP;
        hrd->hdr_checksum16 = 0;
        memcpy(hrd->src_ip, src_ip, NET_IP_LEN);
        memcpy(hrd->dst_ip, net_if_ip, NET_IP_LEN);
        hrd->hdr_checksum16 = (checksum16(buf->data, IP_HDR_LEN));
        // 端口不可达
        icmp_unreachable(buf, src_ip, ICMP_CODE_PORT_UNREACH);
    }
}

/**
 * @brief 处理一个要发送的数据包
 * 
 * @param buf 要处理的包
 * @param src_port 源端口号
 * @param dst_ip 目的ip地址
 * @param dst_port 目的端口号
 */
void udp_out(buf_t *buf, uint16_t src_port, uint8_t *dst_ip, uint16_t dst_port)
{
    // TO-DO
    // printf("udp_out\n");
    // 添加UDP报头
    buf_add_header(buf, sizeof(udp_hdr_t));
    udp_hdr_t *hrd = (udp_handler_t *)buf->data;
    // 配置header
    hrd->src_port16 = swap16(src_port);
    hrd->dst_port16 = swap16(dst_port);
    hrd->total_len16 = swap16(buf->len);
    hrd->checksum16 = 0;
    hrd->checksum16 = udp_checksum(buf, net_if_ip, dst_ip);
    // printf("checksum:%x\n", hrd->checksum16);

    ip_out(buf, dst_ip, NET_PROTOCOL_UDP);
}

/**
 * @brief 初始化udp协议
 * 
 */
void udp_init()
{
    map_init(&udp_table, sizeof(uint16_t), sizeof(udp_handler_t), 0, 0, NULL);
    net_add_protocol(NET_PROTOCOL_UDP, udp_in);
}

/**
 * @brief 打开一个udp端口并注册处理程序
 * 
 * @param port 端口号
 * @param handler 处理程序
 * @return int 成功为0，失败为-1
 */
int udp_open(uint16_t port, udp_handler_t handler)
{
    // printf("udp_open\n");
    return map_set(&udp_table, &port, &handler);
}

/**
 * @brief 关闭一个udp端口
 * 
 * @param port 端口号
 */
void udp_close(uint16_t port)
{
    map_delete(&udp_table, &port);
}

/**
 * @brief 发送一个udp包
 * 
 * @param data 要发送的数据
 * @param len 数据长度
 * @param src_port 源端口号
 * @param dst_ip 目的ip地址
 * @param dst_port 目的端口号
 */
void udp_send(uint8_t *data, uint16_t len, uint16_t src_port, uint8_t *dst_ip, uint16_t dst_port)
{
    buf_init(&txbuf, len);
    memcpy(txbuf.data, data, len);
    udp_out(&txbuf, src_port, dst_ip, dst_port);
}