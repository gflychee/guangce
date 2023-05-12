#ifndef _TRADER_TAC_H_
#define _TRADER_TAC_H_

#include <string>
#include <vector>
#include <map>

#include <winterfell/cfg.h>
#include <winterfell/log.h>
#include <winterfell/trader.h>
#include <winterfell/instab.h>

#include "TacFtdcTraderApi.h"
#include "TacFtdcUserApiStruct.h"

class TraderTac : public CTacFtdcTraderSpi {
public:
	TraderTac(cfg_t *cfg, struct memdb *memdb);
    int login(cfg_t *cfg);
    int logout();
	inline int send_order(struct order *order);
	inline int cancel_order(struct order *order);
	inline void load_config(cfg_t *cfg);

    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected();
    
    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    virtual void OnFrontDisconnected(int nReason);

    ///错误应答
    virtual void OnRspError(CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast); 

    ///App认证应答
    virtual void OnRspAuthenticate(CTacFtdcRspAuthenticateField *pRspAuthenticate, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast); 

    ///用户登录应答
    virtual void OnRspUserLogin(CTacFtdcRspUserLoginField *pRspUserLogin, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///用户登出应答
    virtual void OnRspUserLogout(CTacFtdcUserLogoutField *pUserLogout, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast); 

    ///报单录入应答
    virtual void OnRspOrderInsert(CTacFtdcRspOrderInsertField *pRspOrderInsert, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast); 

    ///报单操作应答
    virtual void OnRspOrderAction(CTacFtdcRspOrderActionField *pRspOrderAction, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///UDP丢包应答
    virtual void OnUdpMsgRej(int udpID);

    ///报单回报
    virtual void OnRtnOrder(CTacFtdcRtnOrderField *pOrder);

    ///成交回报
    virtual void OnRtnTrade(CTacFtdcRtnTradeField *pTrade);

	struct trader trader;
private:
    void init_new_order(CTacFtdcInputOrderField *req);
    void init_cancel(CTacFtdcInputOrderActionField *req);
private:
	CTacFtdcInputOrderField new_order;
	CTacFtdcInputOrderActionField cancel;
	TacApiClientReqId reqid;
	TacApiReferenceType orderref;
	CTacFtdcTraderApi *api;
	cfg_t *cfg;
	TacApiRspLoginField m_LoginInfo;
	TacApiReqLoginField reqUserLogin;
	
    std::map<std::string, TacApiContractIndexType> cNo2CIndex;
	std::map<long, TacApiReferenceType> id2ref;
	std::map<TacApiReferenceType, long> ref2id;
	std::map<TacApiReferenceType, TacApiOrderIdType> ref2orderId; // ref 和 委托号建立索引
    volatile int login_finished;
};

#endif
