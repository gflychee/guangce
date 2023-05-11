#ifndef _TRADER_DSTAR_H_
#define _TRADER_DSTAR_H_

#include <string>
#include <vector>
#include <map>
#include <winterfell/cfg.h>
#include <winterfell/log.h>
#include <winterfell/trader.h>
#include <winterfell/instab.h>

#include "DstarTradeApiError.h"
#include "DstarTradeApiDataType.h"
#include "DstarTradeApiStruct.h"
#include "DstarTradeApi.h"
#include "DSTARTradeApi.h"

class TraderDstar : public IDstarTradeSpi {
public:
	TraderDstar(cfg_t *cfg, struct memdb *memdb);
    int login(cfg_t *cfg);
    int logout();
	inline int send_order(struct order *order);
	inline int cancel_order(struct order *order);
	inline void load_config(cfg_t *cfg);
    inline int query_position(int stid, int insidx);

    ///客户端与接口通信连接断开
    virtual void OnFrontDisconnected();

    ///错误应答
    virtual void OnRspError(DstarApiErrorCodeType nErrorCode);

    ///登录请求响应,错误码为0说明用户登录成功。
    virtual void OnRspUserLogin(const DstarApiRspLoginField *pRspUserLogin);
    
    ///提交信息响应
    virtual void OnRspSubmitInfo(const DstarApiRspSubmitInfoField *pRspSubmitInfo);

    ///合约信息响应
    virtual void OnRspContract(const DstarApiContractField *pContract);

    ///席位信息响应
    virtual void OnRspSeat(const DstarApiSeatField* pSeat);

    ///手续费参数响应
    virtual void OnRspTrdFeeParam(const DstarApiTrdFeeParamField* pFeeParam);

    ///保证金参数响应
    virtual void OnRspTrdMarParam(const DstarApiTrdMarParamField* pMarParam);

    ///市场状态信息响应
    virtual void OnRspTrdExchangeState(const DstarApiTrdExchangeStateField *pTrdExchangeState);
    
    ///资金快照响应
    virtual void OnRspFund(const DstarApiFundField *pFund);

    ///昨持仓快照响应
    virtual void OnRspPrePosition(const DstarApiPrePositionField *pPrePosition);
    
    ///实时持仓快照响应
    virtual void OnRspPosition(const DstarApiPositionField *pPosition);

    ///委托响应
    virtual void OnRspOrder(const DstarApiOrderField *pOrder);
    
    ///报价响应
    virtual void OnRspOffer(const DstarApiOfferField *pOffer);
    
    ///成交响应
    virtual void OnRspMatch(const DstarApiMatchField *pMatch);

    ///出入金响应
    virtual void OnRspCashInOut(const DstarApiCashInOutField *pCashInOut);
    
    ///API准备就绪,用户只有收到此就绪通知时才能进行后续的操作
    ///@param nSerialId 数据快照对应的流号,如果依据快照数据计算,使用该流号之后的数据
    virtual void OnApiReady(const DstarApiSerialIdType nSerialId);
    
    ///UDP认证请求响应,错误码为0说明认证成功。
    virtual void OnRspUdpAuth(const DstarApiRspUdpAuthField *pRspUdpAuth);
    
    ///报单应答
    virtual void OnRspOrderInsert(const DstarApiRspOrderInsertField *pOrderInsert);
    
    ///报价应答
    virtual void OnRspOfferInsert(const DstarApiRspOfferInsertField *pOfferInsert);
    
    ///撤单应答
    virtual void OnRspOrderDelete(const DstarApiRspOrderDeleteField *pOrderDelete);
    
    ///最新请求号应答
    virtual void OnRspLastReqId(const DstaApiRspLastReqIdField *pLastReqId);
    
    ///委托通知 (撤单失败时返回委托通知,委托状态不变,包含撤单失败的错误码)
    virtual void OnRtnOrder(const DstarApiOrderField *pOrder);

    ///成交通知
    virtual void OnRtnMatch(const DstarApiMatchField *pMatch);
    
    ///出入金通知
    virtual void OnRtnCashInOut(const DstarApiCashInOutField *pCashInOut);
    
    ///报价通知 (撤单失败时返回报价通知,报价状态不变,包含撤单失败的错误码)
    virtual void OnRtnOffer(const DstarApiOfferField *pOffer);
    
    ///询价通知
    virtual void OnRtnEnquiry(const DstarApiEnquiryField *pEnquiry);

    ///市场状态通知
    virtual void OnRtnTrdExchangeState(const DstarApiTrdExchangeStateField *pTrdExchangeState);
    
    ///浮盈通知
    virtual void OnRtnPosiProfit(const DstarApiPosiProfitField *pPosiProfit);

    // 密码修改应答
    virtual void OnRspPwdMod(const DstarApiRspPwdModField *pRspPwdModField);

    // 密码修改通知（收到此通知后API会主动断开连接）
    virtual void OnRtnPwdMod(const DstarApiPwdModField *pPwdModField);

    ///席位信息通知
    virtual void OnRtnSeat(const DstarApiSeatField* pSeat);

    ///交易权限响应
    virtual void OnRspTradeRight(const DstarApiTradeRightField* pTradeRight);

    ///客户品种白名单响应
    virtual void OnRspAccountCommList(const DstarApiAccountCommListField* pAccountCommList);

    ///交易权限通知 (当添加或者修改某个品种交易权限时，会推送此通知)
    virtual void OnRtnTradeRight(const DstarApiTradeRightField* pTradeRight);

    ///交易权限删除通知 (当删除某个品种交易权限时，会推送此通知)
    virtual void OnRtnTradeRightDel(const DstarApiTradeRightDelField* pTradeRightDel);

	struct trader trader;
private:
    void init_new_order(DstarApiReqOrderInsertField *req);
    void init_cancel(DstarApiReqOrderDeleteField *req);
private:
	DstarApiReqOrderInsertField new_order;
	DstarApiReqOrderDeleteField cancel;
	DstarApiClientReqId reqid;
	DstarApiReferenceType orderref;
	DSTARTradeApi *api;
	cfg_t *cfg;
	DstarApiRspLoginField m_LoginInfo;
	DstarApiReqLoginField reqUserLogin;
	
    std::map<std::string, DstarApiContractIndexType> cNo2CIndex;
	std::map<long, DstarApiReferenceType> id2ref;
	std::map<DstarApiReferenceType, long> ref2id;
	std::map<DstarApiReferenceType, DstarApiOrderIdType> ref2orderId; // ref 和 委托号建立索引
    std::multimap<int, struct query_position_rtn> pre_position;
    volatile int login_finished;

};

#endif
