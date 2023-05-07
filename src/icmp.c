#include "net.h"
#include "icmp.h"
#include "ip.h"

/**
 * @brief 发送icmp响应
 * 
 * @param req_buf 收到的icmp请求包
 * @param src_ip 源ip地址
 */
static void icmp_resp(buf_t *req_buf, uint8_t *src_ip)
{
    // TO-DO
    // 初始化buf
    buf_init(&txbuf, req_buf->len);       
    memcpy(txbuf.data, req_buf->data, req_buf->len);
    // 原先的头部
    icmp_hdr_t *pre_hrd = (icmp_hdr_t *)req_buf->data;

    icmp_hdr_t *hrd = (icmp_hdr_t *)txbuf.data;
    // printf("icmp_resp len: %d\n", txbuf.len);

    hrd->type = 0;
    hrd->code = 0;

    hrd->checksum16 = 0;
    hrd->checksum16 = checksum16(txbuf.data, txbuf.len);
    // 调用ip_out()发送
    ip_out(&txbuf, src_ip, NET_PROTOCOL_ICMP);
}

/**
 * @brief 处理一个收到的数据包
 * 
 * @param buf 要处理的数据包
 * @param src_ip 源ip地址
 */
void icmp_in(buf_t *buf, uint8_t *src_ip)
{
    printf("icmp_in\n");
    // TO-DO
    if (buf->len < sizeof(icmp_hdr_t)) {
        return;
    }
    // 获取数据
    icmp_hdr_t *hrd = (icmp_hdr_t *)buf->data;
    // 报头检测(是否为回显请求)

    // printf("type:%d\n", hrd->type);
    if (hrd->type == ICMP_TYPE_ECHO_REQUEST)
    {
        // printf("icmp_resp\n");
        icmp_resp(buf, src_ip);
    }
}

/**
 * @brief 发送icmp不可达
 * 
 * @param recv_buf 收到的ip数据包(报头还在)
 * @param src_ip 源ip地址
 * @param code icmp code，协议不可达或端口不可达
 */
void icmp_unreachable(buf_t *recv_buf, uint8_t *src_ip, icmp_code_t code)
{
    printf("icmp_unreachable\n");
    // TO-DO
    // 初始化buf
    buf_init(&txbuf, REMAIN_DATA_LEN);       // icmp数据部分长度为28
    memcpy(txbuf.data, recv_buf->data, REMAIN_DATA_LEN);
    // 添加头部
    buf_add_header(&txbuf, sizeof(icmp_hdr_t));
    icmp_hdr_t *hrd = (icmp_hdr_t *)txbuf.data;

    if (code == ICMP_CODE_PROTOCOL_UNREACH)
    {
        hrd->type = 3;
        hrd->code = 2;
    } else if (code == ICMP_CODE_PORT_UNREACH)
    {
        hrd->type = 3;
        hrd->code = 3;
    }
    
    hrd->id16 = 0;
    hrd->seq16 = 0;    // 差错报文这两个字段未用，需为0
    hrd->checksum16 = 0;
    hrd->checksum16 = checksum16(txbuf.data, txbuf.len);
    // 调用ip_out()发送
    ip_out(&txbuf, src_ip, NET_PROTOCOL_ICMP);
}

/**
 * @brief 初始化icmp协议
 * 
 */
void icmp_init(){
    net_add_protocol(NET_PROTOCOL_ICMP, icmp_in);
}