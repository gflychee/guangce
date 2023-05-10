/////////////////////////////////////////////////////////////////////////
///@system      TacMars交易系统
///@company     上海广策信息技术有限公司
///@file        TacFtdcTraderApi.h
///@brief       定义了客户端接口
///@version     2.0 
/////////////////////////////////////////////////////////////////////////

#if !defined(TAC_FTDCTRADERAPI_H)
#define TAC_FTDCTRADERAPI_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "TacFtdcUserApiStruct.h"

#if defined(IS_LIB) && defined(WIN32)
#ifdef LIB_TRADER_API_EXPORT
#define TRADER_API_EXPORT __declspec(dllexport)
#else
#define TRADER_API_EXPORT __declspec(dllimport)
#endif
#else
#define TRADER_API_EXPORT __attribute__((visibility("default"))) 
#endif

class CTacFtdcTraderSpi
{
public:
#define DEFINE_SPI(function_name,...) virtual void function_name (__VA_ARGS__){}
#include "TacFtdcTraderSpiInterface.h" 
};

class TRADER_API_EXPORT CTacFtdcTraderApi
{
public:
    ///创建TraderApi
    ///@param pszFlowPath 存贮订阅信息文件的目录，默认为当前目录
    ///@return 创建出的TraderApi
    static CTacFtdcTraderApi *CreateFtdcTraderApi(const char *pszFlowPath = ".");

    ///获取API版本号
    ///@return API版本号
    static const char *GetApiVersion();
    
    ///初始化
    ///@remark 初始化运行环境,只有调用后,接口才开始工作
    void Init();

    ///删除接口对象本身
    ///@remark 不再使用本接口对象时,调用该函数删除接口对象
    void Release();
    
    ///等待接口线程结束运行
    ///@return 线程退出代码
    int Join();

    ///注册前置机网络地址
    /// url格式为：“protocol://ip:port”，如：”tcp://127.0.0.1:17001”
    void RegisterFront(const char *pszFrontAddress);
    
    ///注册回调接口
    ///@param pSpi 派生自回调接口类的实例
    void RegisterSpi(CTacFtdcTraderSpi *pSpi);

    ///订阅私有流。
    ///@param nResumeType 私有流重传方式  
    ///        TAC_TERT_RESTART:从本交易日开始重传
    ///        TAC_TERT_RESUME:从上次收到的续传
    ///        TAC_TERT_QUICK:只传送登录后私有流的内容
    ///@remark 该方法要在Init方法前调用。若不调用则不会收到私有流的数据。
    void SubscribePrivateTopic(TAC_TE_RESUME_TYPE nResumeType);

    ///设置日志目录路径
    bool SetLogFilePath(const char* pszLogFilePrefix);

    ///设置日志级别
    void SetLogLevel(bool isLogMsg);

    ///绑定指定的CPU核心
    int SetTraderApiCpuAffinity(int recv_cpu, int send_cpu);

    ///加速发送的平均速度，但是收包延迟会增加，默认开启
    bool SetOpenFastSend(bool isOpen);
    
    ///使用udp协议发送，可以增加发送速度，但是有可能丢包。默认关闭。
    bool SetUdpSendOpen(bool isOpen);

    ///获取当前交易日
    ///@return 获取到的交易日
    ///@remark 只有登录成功后,才能得到正确的交易日
    const char *GetTradingDay();

    ///App认证请求
    int ReqAuthenticate(CTacFtdcAuthenticateField *pAuthenticate, int nRequestID);

    ///用户登录请求
    int ReqUserLogin(CTacFtdcUserLoginField *pUserLoginField, int nRequestID);

    ///用户登出请求
    int ReqUserLogout(CTacFtdcUserLogoutField *pUserLogout, int nRequestID);

    ///报单录入请求，Udp模式下会返回udp计数，用于和OnUdpMsgRej中的值对照
    int ReqOrderInsert(CTacFtdcInputOrderField *pInputOrder, int nRequestID);

    ///报单操作请求，Udp模式下会返回udp计数，用于和OnUdpMsgRej中的值对照
    int ReqOrderAction(CTacFtdcInputOrderActionField *pInputOrderAction, int nRequestID);

    ///合约查询请求
    int ReqQryInstrument(CTacFtdcQryInstrumentField *pQryInstrument, int nRequestID);

    ///资金查询请求
    int ReqQryAccount(CTacFtdcQryAccountField *pQryAccount, int nRequestID);

    ///持仓查询请求
    int ReqQryPosition(CTacFtdcQryPositionField *pQryPosition, int nRequestID);

protected:
    ~CTacFtdcTraderApi();

public:
    ///修改请求发送方式，默认为异步，isAsyncSend为false时，改为同步发送
    ///@param isAsyncSend 是否使用异步发送
    ///@remark 该方法要在Init方法前调用。若isAsyncSend为false，则不会创建发送线程
    void SetUseAsyncRequest(bool isAsyncSend);
};

#endif
