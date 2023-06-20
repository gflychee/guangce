#ifndef TRADE_DEMO_H
#define TRADE_DEMO_H


#include "TacFtdcTraderApi.h"
#include "TacFtdcUserApiStruct.h"

class SimpleHandler : public CTacFtdcTraderSpi
{
public:
    SimpleHandler(CTacFtdcTraderApi* pApi, const char* app, const char* auth, const char* user, const char* pass);

    virtual void OnFrontConnected();

    virtual void OnFrontDisconnected(int nReason);

    ///错误应答
    virtual void OnRspError(CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///用户认证
    virtual void OnRspAuthenticate(CTacFtdcRspAuthenticateField *pRspAuthenticate, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    ///用户登录应答
    virtual void OnRspUserLogin(CTacFtdcRspUserLoginField *pRspUserLogin, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///用户登出应答
    virtual void OnRspUserLogout(CTacFtdcUserLogoutField *pUserLogout, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///报单录入应答
    virtual void OnRspOrderInsert(CTacFtdcRspOrderInsertField *pRspOrderInsert, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) ;

    ///报单回报
    virtual void OnRtnOrder(CTacFtdcRtnOrderField *pOrder) ;

    ///成交回报
    virtual void OnRtnTrade(CTacFtdcRtnTradeField *pTrade) ;

private:
    CTacFtdcTraderApi* m_pApi;
    char m_app[31];
    char m_auth[17];
    char m_user[11];
    char m_pass[41];
};


#endif //TRADE_DEMO_H
