#include <trader_dstar.h>
#include <winterfell/cfg.h>
#include <winterfell/log.h>
#include <winterfell/trader.h>
#include <winterfell/instab.h>
#include <map>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <DSTARTradeApi.h>
#include <DstarTradeApi.h>
#include <DstarTradeApiDataType.h>
#include <DstarTradeApiError.h>
#include <DstarTradeApiStruct.h>


static char *config_read_string(cfg_t *cfg, const char *name)
{
    const char *val = NULL;

    cfg_get_string(cfg, name, &val);

    return strdup(val);
}

TraderDstar::TraderDstar(cfg_t *cfg, struct memdb *memdb) : reqid(0), orderref(0)
{
    char *FrontAddress;
    char *AccountNo;
    char *Password;
    char *AppId;
    char *LicenseNo;
    trader_init(&trader, cfg, memdb);
    memset(&new_order, 0, sizeof(new_order));
    memset(&cancel, 0, sizeof(cancel));

    api = DSTARTradeApi::CreateDSTARTradeApi();
    api->RegisterSpi(this);

    FrontAddress = config_read_string(cfg, "FrontAddress");
    AccountNo = config_read_string(cfg, "AccountNo");
    Password = config_read_string(cfg, "Password");
    AppId = config_read_string(cfg, "AppId");
    LicenseNo = config_read_string(cfg, "LicenseNo");
    strncpy(reqUserLogin.AccountNo, AccountNo, sizeof(reqUserLogin.AccountNo) - 1);
    strncpy(reqUserLogin.Password, Password, sizeof(reqUserLogin.Password) - 1);
    strncpy(reqUserLogin.AppId, AppId, sizeof(reqUserLogin.AppId) - 1);
    strncpy(reqUserLogin.LicenseNo, LicenseNo, sizeof(reqUserLogin.LicenseNo) - 1);
    api->RegisterFront(FrontAddress);
}

int TraderDstar::login(cfg_t *cfg)
{
    login_finished = 0;
    api->SubscribePublicTopic(-1); // -1从最新开始订阅，0从头订阅
    int ret = 0;
    ret = api->ReqUserLogin(&reqUserLogin);
    if (ret) {
        wflog_exit(-1, "login failed");
    }
    return 0;
}

int TraderDstar::logout()
{
    api->Release();
    return 0;
}

int TraderDstar::query_position(int stid, int insidx)
{
    struct query_position_rtn last_position = {0};
    last_position.islast = 1;
    last_position.stid = stid;
    last_position.traderid = trader.traderid;
    last_position.accountid = trader.accountid;
    last_position.posdate = PD_Yesterday;
    last_position.insidx = -1;
    while (login_finished == 0) {
        usleep(1000);
    }
    wflog_msg("query_position, stid:%d, insidx:%d, pre_position.size():%ld", stid, insidx, pre_position.size());
    if (insidx == -1) { // 查询所有仓位
        for (auto& item : pre_position) {
            item.second.stid = stid;
            const char *ins = idx2ins(trader.instab, item.second.insidx);
            wflog_msg("insidx:%d, ins:%s, direction:%d, volume:%d", item.second.insidx, ins, item.second.direction, item.second.volume);
            trader_on_query_position_rtn(&trader, &item.second);
        }
    } else { // 查询指定仓位
        auto iter_range = pre_position.equal_range(insidx);
        if (iter_range.first != std::end(pre_position)) {
            for (auto iter = iter_range.first; iter != iter_range.second; ++iter) {
                iter->second.stid = stid;
                const char *ins = idx2ins(trader.instab, iter->second.insidx);
                wflog_msg("insidx:%d, ins:%s, direction:%d, volume:%d", iter->second.insidx, ins, iter->second.direction, iter->second.volume);
                trader_on_query_position_rtn(&trader, &iter->second);
            }
        }
    }
    trader_on_query_position_rtn(&trader, &last_position);
    return 0;
}

void TraderDstar::OnFrontDisconnected()
{
    wflog_exit(-1, "OnFrontDisconnected");
}

void TraderDstar::OnRspError(DstarApiErrorCodeType nErrorCode)
{
    wflog_err("OnRspError:%u", nErrorCode);
}

void TraderDstar::OnApiReady(const DstarApiSerialIdType nSerialId)
{
    wflog_msg("OnApiReady, serial:%llu", nSerialId);
    DstarApiReqUdpAuthField req;
    req.AccountIndex = m_LoginInfo.AccountIndex;
    req.UdpAuthCode = m_LoginInfo.UdpAuthCode;
    req.ReqIdMode = DSTAR_API_REQIDMODE_NOCHECK;
    api->ReqAuthenticate(&req);
}

void TraderDstar::OnRspUserLogin(const DstarApiRspLoginField *pLoginRsp)
{
    if (pLoginRsp->ErrorCode == 0) {
        login_finished = 1;
        m_LoginInfo = *pLoginRsp;
        wflog_msg("Login");
    } else {
        wflog_exit(-1, "OnRspUserLogin error: %d", pLoginRsp->ErrorCode);
    }
}

void TraderDstar::OnRspSubmitInfo(const DstarApiRspSubmitInfoField *pRspSubmitInfo)
{

}

void TraderDstar::OnRspTrdExchangeState(const DstarApiTrdExchangeStateField *pTrdExchangeState) 
{

}

void TraderDstar::OnRtnOrder(const DstarApiOrderField *pOrder) 
{
    auto it = ref2id.find(pOrder->Reference);
    if (it == ref2id.end()) {
        wflog_msg("OnRtnOrder pOrder->Reference:%lld not found", pOrder->Reference);
        return;
    }

    long orderid = it->second;
    if (pOrder->OrderState == DSTAR_API_STATUS_QUEUE)
        trader_on_send_rtn(&trader, currtime(), orderid, strtol(pOrder->SystemNo, NULL, 10));
    else if (pOrder->OrderState == DSTAR_API_STATUS_DELETE || pOrder->OrderState == DSTAR_API_STATUS_LEFTDELETE)
        trader_on_cancel_rtn(&trader, currtime(), orderid, pOrder->OrderQty - pOrder->MatchQty, strtol(pOrder->SystemNo, NULL, 10));
    else if (pOrder->OrderState == DSTAR_API_STATUS_FAIL)
        trader_on_send_err(&trader, currtime(), orderid, pOrder->ErrCode);
    else {
	if (pOrder->OrderState != DSTAR_API_STATUS_PARTFILL && pOrder->OrderState != DSTAR_API_STATUS_FILL && pOrder->OrderState != DSTAR_API_STATUS_ACCEPT)
            wflog_msg("OnRtnOrder OrderState:%c ErrCode:%d", pOrder->OrderState, pOrder->ErrCode);
    }
}

void TraderDstar::OnRtnOffer(const DstarApiOfferField *pOffer)
{

}

void TraderDstar::OnRtnMatch(const DstarApiMatchField *pMatch)
{
    struct trade trade;
    const int insidx = ins2idx(trader.instab, pMatch->ContractNo);
    if (insidx == -1) {
        wflog_err("OnRtnMatch insidx error");
        return;
    }
    trade.insidx = insidx;
    trade.orderid = ref2id[pMatch->Reference];
    trade.price = pMatch->MatchPrice;
    trade.volume = int(pMatch->MatchQty);
    trade.sysid = strtol(pMatch->SystemNo, NULL, 10);
    trade.recv_time = currtime();
    trader_on_trade_rtn(&trader, &trade);
}

void TraderDstar::OnRtnEnquiry(const DstarApiEnquiryField *pEnquiry)
{

}

void TraderDstar::OnRtnTrdExchangeState(const DstarApiTrdExchangeStateField *pTrdExchangeState) 
{

}

void TraderDstar::OnRtnPosiProfit(const DstarApiPosiProfitField *pPosiProfit)
{

}

void TraderDstar::OnRspSeat(const DstarApiSeatField *pSeatInfo)
{
    wflog_msg("OnRspSeat seatindex:%d exchange:%c seatno:%s seatstate:%c ip:%s", pSeatInfo->SeatIndex, pSeatInfo->Exchange, pSeatInfo->SeatNo, pSeatInfo->SeatState, pSeatInfo->Ip);
}

void TraderDstar::OnRspPwdMod(const DstarApiRspPwdModField *pRspPwdModField)
{

}

void TraderDstar::OnRtnPwdMod(const DstarApiPwdModField *pPwdModField)
{

}

void TraderDstar::OnRtnSeat(const DstarApiSeatField* pSeat)
{

}

void TraderDstar::OnRspTradeRight(const DstarApiTradeRightField* pTradeRight)
{

}

void TraderDstar::OnRspAccountCommList(const DstarApiAccountCommListField* pAccountCommList)
{

}

void TraderDstar::OnRtnTradeRight(const DstarApiTradeRightField* pTradeRight)
{

}

void TraderDstar::OnRtnTradeRightDel(const DstarApiTradeRightDelField* pTradeRightDel)
{

}


void TraderDstar::OnRspContract(const DstarApiContractField *pContract)
{
    cNo2CIndex[pContract->ContractNo] = pContract->ContractIndex;
}

void TraderDstar::OnRspTrdFeeParam(const DstarApiTrdFeeParamField* pFeeParam) 
{

}

void TraderDstar::OnRspTrdMarParam(const DstarApiTrdMarParamField* pMarParam) 
{

}

void TraderDstar::OnRspPrePosition(const DstarApiPrePositionField *pPrePosition)
{
    struct query_position_rtn position = {0};
    position.stid = -1;
    position.traderid = trader.traderid;
    position.accountid = trader.accountid;
    position.posdate = PD_Yesterday;
    position.islast = 0;
    const int insidx = ins2idx(trader.instab, pPrePosition->ContractNo);
    if (insidx == -1) {
        wflog_err("OnRspPrePosition error insidx: %d, ContractNo: %s", insidx, pPrePosition->ContractNo);
        return;
    }
    position.insidx = insidx;
    if (pPrePosition->PreBuyQty > 0 && pPrePosition->PreSellQty > 0) {
        position.direction = D_Buy;
        position.volume = pPrePosition->PreBuyQty;
        pre_position.insert(std::make_pair(position.insidx, position));
        position.direction = D_Sell;
        position.volume = pPrePosition->PreSellQty;
        pre_position.insert(std::make_pair(position.insidx, position));
    } else {
        if (pPrePosition->PreBuyQty > 0 && pPrePosition->PreSellQty == 0) {
            position.direction = D_Buy;
            position.volume = pPrePosition->PreBuyQty;
        } else if (pPrePosition->PreBuyQty == 0 && pPrePosition->PreSellQty > 0){
            position.direction = D_Sell;
            position.volume = pPrePosition->PreSellQty;
        } else {
            wflog_err("OnRspPrePosition error Qty=0");
        }
        pre_position.insert(std::make_pair(position.insidx, position));
    }
}

void TraderDstar::OnRspPosition(const DstarApiPositionField *pPosition)
{

}

void TraderDstar::OnRspFund(const DstarApiFundField *pFund)
{

}

void TraderDstar::OnRspOrder(const DstarApiOrderField *pOrder)
{

}

void TraderDstar::OnRspOffer(const DstarApiOfferField *pOffer)
{

}

void TraderDstar::OnRspMatch(const DstarApiMatchField *pMatch)
{

}

void TraderDstar::OnRspCashInOut(const DstarApiCashInOutField *pCashInOut)
{

}

void TraderDstar::OnRspUdpAuth(const DstarApiRspUdpAuthField *pRspUdpAuth)
{
    if (pRspUdpAuth->ErrorCode == 0) {
        init_new_order(&new_order);
        init_cancel(&cancel);
    } else {
        wflog_exit(-1, "OnRspUdpAuth AccountIndex:%d UdpAuthCode:%u ErrorCode:%u", pRspUdpAuth->AccountIndex, pRspUdpAuth->UdpAuthCode, pRspUdpAuth->ErrorCode);
    }
}

void TraderDstar::OnRspOrderInsert(const DstarApiRspOrderInsertField *pOrderInsert)
{
    if (pOrderInsert->ErrCode) {
        long orderid = ref2id[pOrderInsert->Reference];
        trader_on_send_err(&trader, currtime(), orderid, pOrderInsert->ErrCode);
    } else {
        ref2orderId[pOrderInsert->Reference] = pOrderInsert->OrderId;
    }
}

void TraderDstar::OnRspOfferInsert(const DstarApiRspOfferInsertField *pOfferInsert)
{

}

void TraderDstar::OnRspLastReqId(const DstaApiRspLastReqIdField *pLastReqId)
{

}

void TraderDstar::OnRspOrderDelete(const DstarApiRspOrderDeleteField *pOrderDelete)
{
    if (pOrderDelete->ErrCode) {
        long orderid = ref2id[pOrderDelete->Reference];
        trader_on_cancel_err(&trader, currtime(), orderid, pOrderDelete->ErrCode);
    } else {
        ref2orderId[pOrderDelete->Reference] = pOrderDelete->OrderId;
    }
}

void TraderDstar::OnRtnCashInOut(const DstarApiCashInOutField *pCashInOut)
{

}

static inline char dstar_encode_direction(struct orderflags flags)
{
    const char ret[] = {DSTAR_API_DIRECT_BUY, DSTAR_API_DIRECT_SELL};
    return ret[flags.direction];
}

static inline char dstar_encode_offset(struct orderflags flags)
{
    const char ret[] = {DSTAR_API_OFFSET_OPEN, DSTAR_API_OFFSET_CLOSE, DSTAR_API_OFFSET_CLOSE};
    return ret[flags.offset];
}

static inline char dstar_encode_hedge(struct orderflags flags)
{
    const char ret[] = {DSTAR_API_HEDGE_SPECULATE, DSTAR_API_HEDGE_HEDGE};
    return ret[flags.hedge];
}

static inline char dstar_encode_timecond(struct orderflags flags)
{
    const char ret[] = {DSTAR_API_VALID_GFD, DSTAR_API_VALID_IOC};
    return ret[flags.timecond];
}

void TraderDstar::init_new_order(DstarApiReqOrderInsertField *req)
{
    memset(req, 0, sizeof(*req));
    req->OrderType = DSTAR_API_ORDERTYPE_LIMIT; // 限价单
    req->AccountIndex = m_LoginInfo.AccountIndex;
    req->UdpAuthCode = m_LoginInfo.UdpAuthCode;
    req->MinQty = 0;
}

void TraderDstar::init_cancel(DstarApiReqOrderDeleteField *req)
{
    memset(req, 0, sizeof(*req));
    req->AccountIndex = m_LoginInfo.AccountIndex;
    req->UdpAuthCode = m_LoginInfo.UdpAuthCode;
    req->SystemNo[0] = 0;
}

int TraderDstar::send_order(struct order *order)
{
    const char *ins = idx2ins(trader.instab, order->insidx);
    new_order.Direct = dstar_encode_direction(order->flags);
    new_order.Offset = dstar_encode_offset(order->flags);
    new_order.Hedge = dstar_encode_hedge(order->flags);
    new_order.ValidType = dstar_encode_timecond(order->flags);
    new_order.Reference = ++orderref;
    new_order.ClientReqId = ++reqid;
    new_order.ContractIndex = cNo2CIndex[ins];
    if (order->branchid != -1) {
        new_order.SeatIndex = (unsigned char)order->branchid;
    } else {
        new_order.SeatIndex = 0;
    }

    strncpy(new_order.ContractNo, ins, sizeof(DstarApiContractNoType) - 1);
    new_order.OrderQty = order->volume;
    new_order.OrderPrice = order->price;

    id2ref[order->orderid] = orderref;
    ref2id[orderref] = order->orderid;

    int ret = api->ReqOrderInsert(&new_order);

    return ret;
}

int TraderDstar::cancel_order(struct order *order)
{
    int branchid = trader_get_branchid(&trader);
    cancel.ClientReqId = ++reqid;
    if (branchid != -1) {
        cancel.SeatIndex = (unsigned char)branchid;
    } else {
        cancel.SeatIndex = 0;
    }
    cancel.Reference = id2ref[order->orderid];
    cancel.OrderId = ref2orderId[id2ref[order->orderid]];
    return api->ReqOrderAction(&cancel);
}


void TraderDstar::load_config(cfg_t *cfg)
{

}


static int dstar_send_order(struct trader *trader, struct order *order)
{
    TraderDstar *dstar = (TraderDstar *)trader->container;
    return dstar->send_order(order);
}

static int dstar_cancel_order(struct trader *trader, struct order *order)
{
    TraderDstar *dstar = (TraderDstar *)trader->container;
    return dstar->cancel_order(order);
}

static void dstar_load_config(struct trader *trader, cfg_t *cfg)
{
    TraderDstar *dstar = (TraderDstar *)trader->container;
    dstar->load_config(cfg);
}


static int dstar_login(struct trader *trader, cfg_t *cfg)
{
    TraderDstar *dstar = (TraderDstar *)trader->container;
    return dstar->login(cfg);
}


static int dstar_logout(struct trader *trader)
{
    TraderDstar *dstar = (TraderDstar *)trader->container;
    return dstar->logout();
}


static int dstar_query_position(struct trader *trader, int stid, int insidx)
{
    TraderDstar *dstar = (TraderDstar *)trader->container;
    return dstar->query_position(stid, insidx);
}


static struct trader *dstar_create(cfg_t *cfg, struct memdb *memdb)
{
    TraderDstar *dstar = new TraderDstar(cfg, memdb);
    dstar->trader.container = (void *)dstar;
    dstar->trader.ops.send_order = dstar_send_order;
    dstar->trader.ops.cancel_order = dstar_cancel_order;
    dstar->trader.ops.load_config = dstar_load_config;
    dstar->trader.ops.login = dstar_login;
    dstar->trader.ops.logout = dstar_logout;
    dstar->trader.ops.query_position = dstar_query_position;
    dstar->trader.ops.on_new_md = NULL;
    return &dstar->trader;
}

static struct trader_module trader_dstar = {
    .create = dstar_create,
    .api = "dstar",
};

trader_module_register(&trader_dstar);
