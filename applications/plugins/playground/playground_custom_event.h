#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum {
    // Reserve first 100 events for button types and indexes, starting from 0
    PlayGroundCustomEventTypeReserved = 100,

    PlayGroundCustomEventTypeBackPressed,
    PlayGroundCustomEventTypeIndexSelected,
    PlayGroundCustomEventTypeTransmitStarted,
    PlayGroundCustomEventTypeTransmitContinuousStarted,
    PlayGroundCustomEventTypeTransmitFinished,
    PlayGroundCustomEventTypeTransmitNotStarted,
    PlayGroundCustomEventTypeTransmitCustom,
    PlayGroundCustomEventTypeSaveFile,
    PlayGroundCustomEventTypeUpdateView,
    PlayGroundCustomEventTypeChangeStepUp,
    PlayGroundCustomEventTypeChangeStepDown,
    PlayGroundCustomEventTypeChangeStepUpMore,
    PlayGroundCustomEventTypeChangeStepDownMore,

    PlayGroundCustomEventTypeMenuSelected,
    PlayGroundCustomEventTypeTextEditDone,
    PlayGroundCustomEventTypePopupClosed,

    PlayGroundCustomEventTypeLoadFile,
} PlayGroundCustomEvent;