#include <stdio.h>

#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/bigarray.h>

#include "cxx_wrapped.h"

#include "GrContext.h"
#include "gl/GrGLInterface.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkSurface.h"

typedef wrapped<sk_sp<GLInterface>> wGLInterface;
template <>
char const *ml_name<wGLInterface::type>() { return "sk_sp<GLInterface>" }

typedef wrapped<sk_sp<GrContext>> wGrContext;
template <>
char const *ml_name<wGrContext::type>() { return "sk_sp<GrContext>" }

typedef wrapped<sk_sp<SkSurface>> wSkSurface;
template <>
char const *ml_name<wSkSurface::type>() { return "sk_sp<SkSurface>" }

typedef wrapped<sk_sp<SkImage>> wSkImage;
template <>
char const *ml_name<wSkImage::type>() { return "sk_sp<SkImage>" }

typedef wrapped<sk_sp<SkData>> wSkData;
template <>
char const *ml_name<wSkData::type>() { return "sk_sp<SkData>" }

struct custom_operations ImageInfoCustomOperations = {
    identifier : "SkImageInfo",
    finalize : custom_finalize_default,
    compare : custom_compare_default,
    compare_ext : custom_compare_ext_default,
    hash : custom_hash_default,
    serialize : custom_serialize_default,
    deserialize : custom_deserialize_default,
};

struct custom_operations PaintCustomOperations = {
    identifier : "SkPaint",
    finalize : custom_finalize_default,
    compare : custom_compare_default,
    compare_ext : custom_compare_ext_default,
    hash : custom_hash_default,
    serialize : custom_serialize_default,
    deserialize : custom_deserialize_default,
};

struct custom_operations ColorCustomOperations = {
    identifier : "SkColor",
    finalize : custom_finalize_default,
    compare : custom_compare_default,
    compare_ext : custom_compare_ext_default,
    hash : custom_hash_default,
    serialize : custom_serialize_default,
    deserialize : custom_deserialize_default,
};

extern "C"
{
    CAMLprim value
    caml_SkColor_make(value vAlpha, value vRed, value vGreen, value vBlue)
    {
        CAMLparam4(vAlpha, vRed, vGreen, vBlue);
        CAMLlocal1(vColor);
        vColor = caml_alloc_custom(&ColorCustomOperations, sizeof(SkColor), 0, 1);
        *vColor = SkColorSetARGB(Int_val(vAlpha), Int_val(vRed), Int_val(vGreen), Int_val(vBlue));
        CAMLreturn(vColor);
    }

    CAMLprim value
    caml_SkPaint_make(value vUnit)
    {
        CAMLparam1(vUnit);
        CAMLlocal1(vPaint);
        vPaint = caml_alloc_custom(&PaintCustomOperations, sizeof(SkPaint), 0, 1);
        *vPaint = SkPaint();
        CAMLreturn(vPaint);
    }

    CAMLprim value
    caml_SkPaint_setColor(value vPaint, value vColor)
    {
        CAMLparam2(vPaint, vColor);
        SkPaint *pPaint = (SkPaint *)Data_custom_val(vPaint);
        SkColor *pColor = (SkColor *)Data_custom_val(vColor);
        pPaint->setColor(*pColor);
    }

    SkBudgeted variantToBudgeted(value vBudgeted)
    {
        switch (Int_val(vBudgeted))
        {
        case 0:
            return SkBudgeted::kNo;
        case 1:
            return SkBudgeted::kYes;
        default:
            warn("Unexpected option for budgeted");
            return 0;
        }
    }

    SkColorType variantToColorType(value vColorType)
    {
        switch (Int_val(vColorType))
        {
        case 0:
            return kUnknown_SkColorType;
        case 1:
            return kAlpha_8_SkColorType;
        case 2:
            return kRGB_565_SkColorType;
        case 3:
            return kARGB_4444_SkColorType;
        case 4:
            return kRGBA_8888_SkColorType;
        case 5:
            return kRGB_888x_SkColorType;
        case 6:
            return kBGRA_8888_SkColorType;
        case 7:
            return kRGBA_1010102_SkColorType;
        case 8:
            return kRGB_101010x_SkColorType;
        case 9:
            return kGray_8_SkColorType;
        case 10:
            return kRGBA_F16Norm_SkColorType;
        case 11:
            return kRGBA_F16_SkColorType;
        case 12:
            return kRGBA_F32_SkColorType;
        case 13:
            return kN32_SkColorType;
        default:
            warn("Unexpected option for colorType");
            return 0;
        }
    }

    CAMLprim value
    caml_GrContext_MakeGL(value vGLInterfaceOption)
    {
        CAMLparam1(vGlInterfaceOption);
        sk_sp<GrContext> context = vGLInterfaceOption == Val_none
                                       ? GrContext::MakeGL(nullptr)
                                       : GrContext::MakeGL(wGLInterface::get(Val_some(vGLInterface)));
        CAMLreturn(wGrContext::alloc(context));
    }

    CAMLprim value
    caml_SkImageInfo_Make(value vWidth, value vHeight, value vColorType, value vAlphaType, value vColorSpaceOption)
    {
        CAMLparam5(vWidth, vHeight, vColorType, vAlphaType, vColorSpaceOption);
        CAMLlocal1(vImageInfo);
        vImageInfo = caml_alloc_custom(&ImageInfoCustomOperations, sizeof(SkImageInfo), 0, 1);
        SkImageInfo *pImageInfo = (SkImageInfo *)Data_custom_val(vImageInfo);
        *pImageInfo = vSkColorSpaceOption == Val_none
                          ? SkImageInfo::Make(
                                Int_val(vWidth),
                                Int_val(vHeight),
                                variantToColorType(vColorType),
                                variantToAlphaType(vAlphaType))
                          : SkImageInfo::Make(
                                Int_val(vWidth),
                                Int_val(vHeight),
                                variantToColorType(vColorType),
                                variantToAlphaType(vAlphaType),
                                wColorSpace::get(Val_some(vColorSpaceOption)));
        CAMLreturn(vImageInfo);
    }

    CAMLprim value
    caml_SkSurface_MakeRenderTarget(value vContext, value vBudgeted, value vImageInfo)
    {
        CAMLparam3(vContext, vBudgeted, vImageinfo);
        sk_sp<GrContext> context = wGrContext::get(vContext);
        SkBudgeted budgeted = variantToBudgeted(vBudgeted);
        SkImageInfo pImageInfo = (SkImageInfo *)Data_custom_val(vImageInfo);
        sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(context, budgeted, *pImageInfo);
        CAMLreturn(wSkSurface::alloc(surface));
    }

    CAMLprim value
    caml_SkSurface_getCanvas(value vSurface)
    {
        CAMLparam1(vSurface);
        sk_sp<SkSurface> surface = wSkSurface::get(vSurface);
        SkCanvas *pCanvas = surface->getCanvas();
        CAMLreturn((value)pCanvas);
    }

    CAMLprim value
    caml_SkSurface_makeImageSnapshot(value vSurface)
    {
        CAMLparam1(vSurface);
        sk_sp<SkSurface> surface = wSkSurface::get(vSurface);
        sk_sp<SkImage> image = surface->makeImageSnapshot();
        CAMLreturn(wSkImage::alloc(image));
    }

    CAMLprim value
    caml_SkCanvas_drawPaint(value vCanvas, value vPaint)
    {
        CAMLparam2(vCanvas, vPaint);
        Canvas *pCanvas = (SkCanvas *)vCanvas;
        SkPaint *pPaint = (SkPaint *)Data_custom_val(vPaint);
        pCanvas->drawPaint(*pPaint);
        CAMLreturn(Val_unit);
    }

    CAMLprim value
    caml_SkImage_encodeToData(value vImage)
    {
        CAMLparam1(vImage);
        sk_sp<SkImage> image = wImage::get(vImage);
        sk_sp<SkData> data = image->encodeToData();
        CAMLreturn(wData::alloc(data));
    }

    CAMLprim value
    caml_SkFILEWStream_make(value vPath)
    {
        CAMLparam1(vPath);
        CAMLlocal1(vFILEWStream);
        vFILEWStream = caml_alloc_custom(&FILEWStreamCustomOperations, sizeof(SkFILEWStream), 0, 1);
        *vFILEWStream = SkFILEWStream(String_val(vPath));
        CAMLreturn(vFILEWSream);
    }

    CAMLprim value
    caml_SkFILEWStream_write(value vFILEWStream, value vData)
    {
        CAMLparam2(vFILEWStream, vData);
        SkFILEWStream *fileWStream = (SkFILEWStream *)Data_custom_val(vFILEWStream);
        sk_sp<SkData> data = wData::get(vData);
        fileWStream->write(data->data(), data->size());
    }
}