#include <map>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "trader_tac.h"


static char *config_read_string(cfg_t *cfg, const char *name)
{
    const char *val = NULL;

    cfg_get_string(cfg, name, &val);

    return strdup(val);
}

TraderTac::TraderTac(cfg_t *cfg, struct memdb *memdb) : orderref(0)
{
    trader_init(&trader, cfg, memdb);

    frontaddress = config_read_string(cfg, "frontaddress");
    userid = config_read_string(cfg, "userid");
    password = config_read_string(cfg, "password");
    appid = config_read_string(cfg, "appid");
    authcode = config_read_string(cfg, "authcode");
    exchange = config_read_string(cfg, "exchange");
    clientlogfilepath = config_read_string(cfg, "clientlogfilepath");

    api->RegisterFront(frontaddress);
}

int TraderTac::login(cfg_t *cfg)
{
    login_finished = 0;
    
    api = CTacFtdcTraderApi::CreateFtdcTraderApi("flow");
    
    if (api == nullptr)
    {
        wflog_exit(-1, "Create CTacFtdcTraderApi failed");
    }
    wflog_msg("version:%s",api->GetApiVersion());
    api->RegisterSpi(this);
    api->RegisterFront(frontaddress);
    api->SubscribePrivateTopic(TAC_TERT_QUICK);
    api->SetLogFilePath(clientlogfilepath);
    api->SetLogLevel(true);
    api->SetTraderApiCpuAffinity(-1, 1); // recv_cpu 设为-1 不绑定cpu, send_cpu 设为大于0 的数，说明绑定cpu
    api->SetUdpSendOpen(true); // 使用udp协议发送，可以增加发送速度，但是有可能丢包。

    // Start worker thread and init api after setup configuration
    api->Init();

    while (!login_finished)
		usleep(1000);

    return 0;
}

int TraderTac::logout()
{
    api->Release();
    return 0;
}

void TraderTac::OnFrontConnected()
{
    wflog_msg("Front %s connected", FrontAddress);

    CTacFtdcAuthenticateField req = {};

    // Set APP ID
    strncpy(req.AppID, appid, sizeof(req.AppID));

    // Set Authenticate Code
    strncpy(req.AuthCode, authcode, sizeof(req.AuthCode));

    // Set User ID
    strncpy(req.UserID, userid, sizeof(req.UserID));

    // Submit Login request
    api->ReqAuthenticate(&req, 0);
}

void TraderTac::OnFrontDisconnected(int nReason)
{
    wflog_exit(-1, "OnFrontDisconnected:%d", nReason);
}

void TraderTac::OnRspError(CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != nullptr) {
        wflog_err("OnRspError,ErrorID:%d,ErrorMsg:%s,nRequestID:%d,bIsLast:%d", pRspInfo->ErrorID, pRspInfo->ErrorMsg, nRequestID, bIsLast);
    }
}

void TraderTac::OnRspAuthenticate(CTacFtdcRspAuthenticateField *pRspAuthenticate, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        wflog_exit(-1, "OnRspAuthenticate Failed:%d", pRspInfo->ErrorID);
    }

    CTacFtdcUserLoginField req = {};

    // Set Client ID
    strncpy(req.UserID, userid, sizeof(req.UserID));

    // Set Client password
    strncpy(req.Password, password, sizeof(req.Password));

    // Submit Login request
    api->ReqUserLogin(&req, 0);
}

///用户登录应答
void TraderTac::OnRspUserLogin(CTacFtdcRspUserLoginField *pRspUserLogin, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0)
    {
        wflog_exit(-1, "OnRspUserLogin Failed,ErrorID:%d,ErrorMsg:%s", pRspInfo->ErrorID,pRspInfo->ErrorMsg);
    }
    orderref = pRspUserLogin->MaxOrderRef;

    init_new_order(&new_order);
    init_cancel(&cancel);

    login_finished = 1;

    wflog_msg("Login");
}

///用户登出应答
void TraderTac::OnRspUserLogout(CTacFtdcUserLogoutField *pUserLogout, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    wflog_exit(-1, "OnRspUserLogout,ErrorID:%d,ErrorMsg:%s", pRspInfo->ErrorID,pRspInfo->ErrorMsg);
}

///UDP丢包应答
void TraderTac::OnUdpMsgRej(int udpID)
{
    wflog_msg("OnUdpMsgRej,udpID:%d", udpID);
}

///报单录入应答  失败和成功都只有一次
void TraderTac::OnRspOrderInsert(CTacFtdcRspOrderInsertField *pRspOrderInsert, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    long orderid = ref2id[pRspOrderInsert->OrderRef];
    if (pRspInfo->ErrorID == 0) {
        trader_on_send_rtn(&trader, currtime(), orderid, pRspOrderInsert->OrderSysID);
    } else {
        trader_on_send_err(&trader, currtime(), orderid, pRspInfo->ErrorID);
    }
}

///报单操作应答  失败才会有，成功不会有
void TraderTac::OnRspOrderAction(CTacFtdcRspOrderActionField *pRspOrderAction, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo->ErrorID != 0) {
        long orderid = ref2id[pRspOrderAction->OrderRef];
        trader_on_cancel_err(&trader, currtime(), orderid, pRspInfo->ErrorID);
    }
}

///报单回报
void TraderTac::OnRtnOrder(CTacFtdcRtnOrderField *pOrder) 
{
    auto it = ref2id.find(pOrder->OrderRef);
    if (it == ref2id.end()) {
        wflog_msg("OnRtnOrder pOrder->OrderRef:%lu not found", pOrder->OrderRef);
        return;
    }

    long orderid = it->second;
    if (pOrder->OrderStatus == TAC_FTDC_OST_Canceled) {
        trader_on_cancel_rtn(&trader, currtime(), orderid, pOrder->VolumeTotalOriginal - pOrder->VolumeTraded, pOrder->OrderSysID);
    }
}

///成交回报
void TraderTac::OnRtnTrade(CTacFtdcRtnTradeField *pTrade)
{
    struct trade trade;
    const int insidx = ins2idx(trader.instab, pTrade->InstrumentID);
    if (insidx == -1) {
        wflog_err("OnRtnTrade insidx error");
        return;
    }
    trade.insidx = insidx;
    trade.orderid = ref2id[pTrade->OrderRef];
    trade.price = pTrade->Price;
    trade.volume = pTrade->Volume;
    trade.sysid = pTrade->OrderSysID;
    trade.recv_time = currtime();
    trader_on_trade_rtn(&trader, &trade);
}

static inline char tac_encode_direction(struct orderflags flags)
{
    const char ret[] = {TAC_FTDC_D_Buy, TAC_FTDC_D_Sell};
    return ret[flags.direction];
}

static inline char tac_encode_offset(struct orderflags flags)
{
    const char ret[] = {TAC_FTDC_OF_Open, TAC_FTDC_OF_CloseToday, TAC_FTDC_OF_CloseYesterday};
    return ret[flags.offset];
}

static inline char tac_encode_hedge(struct orderflags flags)
{
    const char ret[] = {TAC_FTDC_HF_Speculation, TAC_FTDC_HF_Hedge};
    return ret[flags.hedge];
}

static inline char tac_encode_timecond(struct orderflags flags)
{
    const char ret[] = {TAC_FTDC_TC_GFD, TAC_FTDC_TC_FAK}; // TAC_FTDC_TC_FOK
    return ret[flags.timecond];
}

static inline unsigned char ees_encode_exchange(std::string exchange)
{
    if (exchange == "DCE") {
        return TAC_FTDC_EXID_DCE;
    } else if (exchange == "SHFE") {
        return TAC_FTDC_EXID_SHFE;
    } else if (exchange == "CZCE") {
        return TAC_FTDC_EXID_ZCE;
    } else if (exchange == "CFFEX") {
        return TAC_FTDC_EXID_CFFEX;
    } else if (exchange == "INE") {
        return TAC_FTDC_EXID_INE;
    } else {
        return TAC_FTDC_EXID_SHFE;
    }
}


void TraderTac::init_new_order(CTacFtdcInputOrderField *req)
{
    memset(req, 0, sizeof(*req));
    req->OrderPriceType = TAC_FTDC_OPT_LimitPrice; // 限价单
    req->ExchangeID = ees_encode_exchange(exchange);
    strcpy(req->ClientID, userid);
}

void TraderTac::init_cancel(CTacFtdcInputOrderActionField *req)
{
    memset(req, 0, sizeof(*req));
    strcpy(req->ClientID, userid);
}

int TraderTac::send_order(struct order *order)
{
    const char *ins = idx2ins(trader.instab, order->insidx);
    new_order.Direction = tac_encode_direction(order->flags);
    new_order.OffsetFlag = tac_encode_offset(order->flags);
    new_order.HedgeFlag = tac_encode_hedge(order->flags);
    new_order.TimeCondition = tac_encode_timecond(order->flags);
    new_order.OrderRef = ++orderref;

    strncpy(new_order.InstrumentID, ins, sizeof(TTacFtdcInstrumentIDType) - 1);
    new_order.VolumeTotalOriginal = order->volume;
    new_order.LimitPrice = order->price;

    id2ref[order->orderid] = orderref;
    ref2id[orderref] = order->orderid;

    int ret = api->ReqOrderInsert(&new_order, 0);

    return ret;
}

int TraderTac::cancel_order(struct order *order)
{
    cancel.OrderRef = id2ref[order->orderid];
    //cancel.OrderLocalID = id2ref[order->orderid];
    return api->ReqOrderAction(&cancel, 0);
}


void TraderTac::load_config(cfg_t *cfg)
{

}


static int tac_send_order(struct trader *trader, struct order *order)
{
    TraderTac *tac = (TraderTac *)trader->container;
    return tac->send_order(order);
}

static int tac_cancel_order(struct trader *trader, struct order *order)
{
    TraderTac *tac = (TraderTac *)trader->container;
    return tac->cancel_order(order);
}

static void tac_load_config(struct trader *trader, cfg_t *cfg)
{
    TraderTac *tac = (TraderTac *)trader->container;
    tac->load_config(cfg);
}


static int tac_login(struct trader *trader, cfg_t *cfg)
{
    TraderTac *tac = (TraderTac *)trader->container;
    return tac->login(cfg);
}


static int tac_logout(struct trader *trader)
{
    TraderTac *tac = (TraderTac *)trader->container;
    return tac->logout();
}

static struct trader *tac_create(cfg_t *cfg, struct memdb *memdb)
{
    TraderTac *tac = new TraderTac(cfg, memdb);
    tac->trader.container = (void *)tac;
    tac->trader.ops.send_order = tac_send_order;
    tac->trader.ops.cancel_order = tac_cancel_order;
    tac->trader.ops.load_config = tac_load_config;
    tac->trader.ops.login = tac_login;
    tac->trader.ops.logout = tac_logout;
    tac->trader.ops.query_position = NULL;
    tac->trader.ops.on_new_md = NULL;
    return &tac->trader;
}

static struct trader_module trader_tac = {
    .create = tac_create,
    .api = "tac",
};

trader_module_register(&trader_tac);
