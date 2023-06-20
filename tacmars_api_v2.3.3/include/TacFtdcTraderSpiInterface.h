#ifdef DEFINE_SPI
    ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    DEFINE_SPI (OnFrontConnected)
    
    ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    DEFINE_SPI (OnFrontDisconnected,int nReason)

    ///错误应答
    DEFINE_SPI (OnRspError,CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///App认证应答
    DEFINE_SPI (OnRspAuthenticate,CTacFtdcRspAuthenticateField *pRspAuthenticate, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///用户登录应答
    DEFINE_SPI (OnRspUserLogin,CTacFtdcRspUserLoginField *pRspUserLogin, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///用户登出应答
    DEFINE_SPI (OnRspUserLogout,CTacFtdcUserLogoutField *pUserLogout, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///报单录入应答
    DEFINE_SPI (OnRspOrderInsert,CTacFtdcRspOrderInsertField *pRspOrderInsert, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///报单操作应答
    DEFINE_SPI (OnRspOrderAction,CTacFtdcRspOrderActionField *pRspOrderAction, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///UDP丢包应答
    DEFINE_SPI (OnUdpMsgRej, int udpID) 

    ///报单回报
    DEFINE_SPI (OnRtnOrder,CTacFtdcRtnOrderField *pOrder) 

    ///成交回报
    DEFINE_SPI (OnRtnTrade,CTacFtdcRtnTradeField *pTrade) 

    ///合约查询应答
    DEFINE_SPI (OnRspQryInstrument,CTacFtdcInstrumentField *pInstrument, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///资金查询应答
    DEFINE_SPI (OnRspQryAccount,CTacFtdcAccountField *pAccount, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///持仓查询应答
    DEFINE_SPI (OnRspQryPosition,CTacFtdcPositionField *pPosition, CTacFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 

    ///用户保证金优惠回报
    DEFINE_SPI (OnRtnClientDiscount,CTacFtdcClientFundDiscount* pInfo) 

    ///合约保证金优惠回报
    DEFINE_SPI (OnRtnInstrumentDiscount,CTacFtdcInstrumentDiscount* pInfo) 

#undef DEFINE_SPI
#endif