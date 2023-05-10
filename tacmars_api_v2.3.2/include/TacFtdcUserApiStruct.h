/////////////////////////////////////////////////////////////////////////
///@system      TacMars交易系统
///@company     上海广策信息技术有限公司
///@file        TacFtdcUserApiStruct.h
///@brief       定义了客户端接口使用的业务数据结构
///@version     2.0 
/////////////////////////////////////////////////////////////////////////

#if !defined(TAC_FTDCSTRUCT_H)
#define TAC_FTDCSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "TacFtdcUserApiDataType.h"

///响应信息
struct CTacFtdcRspInfoField
{
    ///错误代码
    TTacFtdcErrorIDType         ErrorID;
    ///错误信息
    TTacFtdcErrorMsgType        ErrorMsg;
};

///App认证请求
struct CTacFtdcAuthenticateField
{
    ///AppID
    TTacFtdcAppIDType           AppID;
    ///授权码
    TTacFtdcAuthCodeType        AuthCode;
    ///用户代码
    TTacFtdcUserIDType          UserID;
};

///App认证应答
struct CTacFtdcRspAuthenticateField
{
    ///AppID
    TTacFtdcAppIDType           AppID;
    ///用户代码
    TTacFtdcUserIDType          UserID;
};

///用户登录请求
struct CTacFtdcUserLoginField
{
    ///用户代码
    TTacFtdcUserIDType          UserID;
    ///密码
    TTacFtdcPasswordType        Password;
    //是否启用回单优化
    TTacFtdcUdpResponseType     EnableUdpResponse;
    //upd端口
    TTacFtdcUdpPortType         UdpPort;
    //udp ip
    TTacFtdcUdpIPType           UdpIP;
    // udp mac addr
    TTacFtdcUdpMacType          UdpMac;
};

///用户登录应答
struct CTacFtdcRspUserLoginField
{
    ///用户代码
    TTacFtdcUserIDType          UserID;
    ///最大报单引用
    TTacFtdcOrderRefType        MaxOrderRef;
    ///交易系统名称
    TTacFtdcSystemNameType      SystemName;
    ///交易日
    TTacFtdcDateType            TradingDay;
    ///登录成功时间
    TTacFtdcTimeType            LoginTime;
};

///用户登出请求
struct CTacFtdcUserLogoutField
{
    ///用户代码
    TTacFtdcUserIDType          UserID;
};

///输入报单
struct CTacFtdcInputOrderField
{
    ///合约代码
    TTacFtdcInstrumentIDType    InstrumentID;
    ///报单价格条件
    TTacFtdcOrderPriceTypeType  OrderPriceType;
    ///买卖方向
    TTacFtdcDirectionType       Direction;
    ///开平标志
    TTacFtdcOffsetFlagType      OffsetFlag;
    ///价格
    TTacFtdcPriceType           LimitPrice;
    ///数量
    TTacFtdcVolumeType          VolumeTotalOriginal;
    ///有效期类型
    TTacFtdcTimeConditionType   TimeCondition;
    ///报单引用
    TTacFtdcOrderRefType        OrderRef;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
    ///投机套保标志
    TTacFtdcHedgeFlagType       HedgeFlag;
    ///交易所代码
    TTacFtdcExchangeIDType      ExchangeID;
};

///报单应答
struct CTacFtdcRspOrderInsertField
{
    ///报单引用
    TTacFtdcOrderRefType        OrderRef;
    ///柜台订单号
    TTacFtdcOrderRefType        OrderLocalID;
    ///交易所订单号
    TTacFtdcOrderRefType        OrderSysID;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
};

///报单操作
struct CTacFtdcInputOrderActionField
{
    ///原报单引用
    TTacFtdcOrderRefType        OrderRef;
    ///原柜台订单号
    TTacFtdcOrderRefType        OrderLocalID;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
};

///报单操作应答
struct CTacFtdcRspOrderActionField
{
    ///报单引用
    TTacFtdcOrderRefType        OrderRef;
    ///柜台订单号
    TTacFtdcOrderRefType        OrderLocalID;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
};

///报单回报
struct CTacFtdcRtnOrderField
{
    ///交易所订单号
    TTacFtdcOrderRefType        OrderSysID;
    ///柜台报单号
    TTacFtdcOrderRefType        OrderLocalID;
    ///报单引用
    TTacFtdcOrderRefType        OrderRef;
    ///报单状态
    TTacFtdcOrderStatusType     OrderStatus;
    ///成交数量
    TTacFtdcVolumeType          VolumeTraded;
    ///剩余数量
    TTacFtdcVolumeType          VolumeTotal;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
    ///合约代码
    TTacFtdcInstrumentIDType    InstrumentID;
    ///报单价格条件
    TTacFtdcOrderPriceTypeType  OrderPriceType;
    ///买卖方向
    TTacFtdcDirectionType       Direction;
    ///开平标志
    TTacFtdcOffsetFlagType      OffsetFlag;
    ///投机套保标志
    TTacFtdcHedgeFlagType       HedgeFlag;
    ///价格
    TTacFtdcPriceType           LimitPrice;
    ///有效期类型
    TTacFtdcTimeConditionType   TimeCondition;
    ///报单数量
    TTacFtdcVolumeType          VolumeTotalOriginal;
    ///交易所代码
    TTacFtdcExchangeIDType      ExchangeID;
};

///成交回报
struct CTacFtdcRtnTradeField
{
    ///成交编号
    TTacFtdcOrderRefType        TradeID;
    ///交易所订单号
    TTacFtdcOrderRefType        OrderSysID;
    ///柜台报单号
    TTacFtdcOrderRefType        OrderLocalID;
    ///报单引用
    TTacFtdcOrderRefType        OrderRef;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
    ///合约代码
    TTacFtdcInstrumentIDType    InstrumentID;
    ///买卖方向
    TTacFtdcDirectionType       Direction;
    ///开平标志
    TTacFtdcOffsetFlagType      OffsetFlag;
    ///投机套保标志
    TTacFtdcHedgeFlagType       HedgeFlag;
    ///成交价格
    TTacFtdcPriceType           Price;
    ///数量
    TTacFtdcVolumeType          Volume;
    ///交易所代码
    TTacFtdcExchangeIDType      ExchangeID;
};

///合约查询请求
struct CTacFtdcQryInstrumentField
{
    ///交易所代码
    TTacFtdcExchangeIDType      ExchangeID;
    ///合约代码
    TTacFtdcInstrumentIDType    InstrumentID;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
};

///合约信息
struct CTacFtdcInstrumentField
{
    ///合约代码
    TTacFtdcInstrumentIDType    InstrumentID;
    ///交易所代码
    TTacFtdcExchangeIDType      ExchangeID;
    ///限价单最大下单量
    TTacFtdcVolumeType          MaxLimitOrderVolume;
    ///限价单最小下单量
    TTacFtdcVolumeType          MinLimitOrderVolume;
    ///合约数量乘数
    TTacFtdcVolumeMultipleType  VolumeMultiple;
    ///最小变动价位
    TTacFtdcPriceType           PriceTick;
    ///前结算价
    TTacFtdcPriceType           PreSettlementPrice;
    ///涨停价
    TTacFtdcPriceType           UpperLimitPrice;
    ///跌停价
    TTacFtdcPriceType           LowerLimitPrice;
    ///多头保证金率
    TTacFtdcRatioType           LongMarginRatio;
    ///空头保证金率
    TTacFtdcRatioType           ShortMarginRatio;
    ///是否使用大额单边保证金算法
    TTacFtdcBoolType            MaxMarginSideAlgorithm;
    ///开仓手续费率
    TTacFtdcRatioType           OpenRatioByMoney;
    ///平仓手续费率
    TTacFtdcRatioType           CloseRatioByMoney;
    ///平今仓手续费率
    TTacFtdcRatioType           CloseTodayRatioByMoney;
    ///开仓手续费率(按手)
    TTacFtdcRatioType           OpenRatioByVolume;
    ///平仓手续费率(按手)
    TTacFtdcRatioType           CloseRatioByVolume;
    ///平今仓手续费率(按手)
    TTacFtdcRatioType           CloseTodayRatioByVolume;
};

///资金账户查询请求
struct CTacFtdcQryAccountField
{
    ///客户代码
    TTacFtdcClientIDType        ClientID;
};

///资金账户信息
struct CTacFtdcAccountField
{
    ///账户
    TTacFtdcClientIDType        ClientID;
    ///初始资金
    TTacFtdcMoneyType           InitFund;
    ///冻结的保证金
    TTacFtdcMoneyType           FrozenMargin;
    ///冻结的手续费
    TTacFtdcMoneyType           FrozenFee;
    ///当前的保证金
    TTacFtdcMoneyType           CurrMargin;
    ///手续费
    TTacFtdcMoneyType           Fee;
    ///平仓盈亏
    TTacFtdcMoneyType           CloseProfit;
    ///持仓盈亏
    TTacFtdcMoneyType           PositionProfit;
    ///可用资金
    TTacFtdcMoneyType           Available;
    ///权益
    TTacFtdcMoneyType           Balance;
    ///冻结的权利金
    TTacFtdcMoneyType           FrozenPremium;
    ///权利金
    TTacFtdcMoneyType           Premium;
    ///保证金优惠额
    TTacFtdcMoneyType           Discount;
};

///客户持仓查询请求
struct CTacFtdcQryPositionField
{
    ///客户代码
    TTacFtdcClientIDType        ClientID;
    ///合约代码
    TTacFtdcInstrumentIDType    InstrumentID;
};

///客户持仓信息
struct CTacFtdcPositionField
{
    ///合约代码
    TTacFtdcInstrumentIDType    InstrumentID;
    ///客户代码
    TTacFtdcClientIDType        ClientID;
    ///投机套保标志
    TTacFtdcHedgeFlagType       HedgeFlag;
    ///持仓多空方向
    TTacFtdcPosiDirectionType   PosiDirection;
    ///上日持仓
    TTacFtdcVolumeType          YdPosition;
    ///今日持仓
    TTacFtdcVolumeType          Position;
    ///开仓冻结
    TTacFtdcVolumeType          LongFrozen;
    ///平仓冻结
    TTacFtdcVolumeType          ShortFrozen;
    ///持仓保证金
    TTacFtdcMoneyType           PositionMargin;
    ///持仓保证金优惠额
    TTacFtdcMoneyType           Discount;
};


struct CTacFtdcClientFundDiscount
{
    ///客户代码
    TTacFtdcClientIDType    ClientID;
    ///客户总保证金
    TTacFtdcMoneyType       TotalUsedMargin;
    ///客户保证金优惠额
    TTacFtdcMoneyType       TotalDiscount;
};

struct CTacFtdcInstrumentDiscount
{
    ///客户代码
    TTacFtdcClientIDType     ClientID;
    ///合约代码
    TTacFtdcInstrumentIDType InstrumentID;
    ///多头单腿持仓
    TTacFtdcVolumeType       SingleLegLongPosition;
    ///空头单腿持仓
    TTacFtdcVolumeType       SingleLegShortPosition;
    ///多头组合持仓
    TTacFtdcVolumeType       CombineLongPosition;
    ///空头组合持仓
    TTacFtdcVolumeType       CombineShortPosition;
    ///多头占用保证金
    TTacFtdcMoneyType        TotalLongUsedMargin;
    ///多头保证金优惠额
    TTacFtdcMoneyType        TotalLongDiscount;
    ///空头占用保证金
    TTacFtdcMoneyType        TotalShortUsedMargin;
    ///空头保证金优惠额
    TTacFtdcMoneyType        TotalShortDiscount;
};



#endif
