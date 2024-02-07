#pragma once
#include<map>
#include <memory>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <mutex>
#include <queue>
using boost::asio::ip::tcp;
#define MAX_LENGTH (1024*2)
#define HEAD_LENGTH 2
class CServer;
class MsgNode;
class CSession: public std::enable_shared_from_this<CSession>
{
private:
    boost::asio::ip::tcp::socket _socket;
    enum {max_length = 1024};
    char _data[MAX_LENGTH]; 
    CServer* _server;
    std::string _uuid;
    std::queue<std::shared_ptr<MsgNode>> _send_que;    
    std::mutex _send_lock;
    //收到的消息体结构
    std::shared_ptr<MsgNode> _recv_msg_node;
    bool _b_head_parse;
    //收到的头部结构
    std::shared_ptr<MsgNode> _recv_head_node;
public:
    CSession(boost::asio::io_context& ioc,CServer* server):_socket(ioc),_data{0},_server(server),_b_head_parse(false),_recv_msg_node(new MsgNode(MAX_LENGTH)),_recv_head_node(new MsgNode(HEAD_LENGTH)){
        //雪花算法  
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }
    tcp::socket& Socket(){
        return _socket;
    }
    void Send(char* msg,int max_length);
    void Start();
    std::string& GetUuid();
private:
    void HandleRead(const boost::system::error_code& ec,size_t bytes_transferred,std::shared_ptr<CSession> _self_CSession);
    void HandleWrite(const boost::system::error_code&ec,std::shared_ptr<CSession> _self_CSession);
};

class MsgNode
{
    friend class CSession;
public:
    //发送
    MsgNode(const char* msg,short total_len):_total_len(total_len+HEAD_LENGTH),_cur_len(0),_msg(nullptr)
    {
        _msg = new char[_total_len+1]();
        memcpy(_msg,&total_len,HEAD_LENGTH);
        memcpy(_msg+HEAD_LENGTH,msg,total_len);
        _msg[_total_len] = 0;
    }
    //接收
    MsgNode(short total_len):_total_len(total_len),_cur_len(0),_msg(nullptr)
    {
        _msg = new char[_total_len + 1]();
    }
    
    ~MsgNode()
    {
        if(!_msg)
        {
            delete[] _msg;
            _msg = nullptr;
        }
    }
    void Clear()
    {
        memset(_msg,0,_total_len);
        _cur_len = 0;
    }
public:
    //总长度
    int _total_len;
    //当前长度
    int _cur_len;
    //消息首地址
    char* _msg;    
};