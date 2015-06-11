/*

 MagickText
 openfx-arena - https://github.com/olear/openfx-arena

 Copyright (c) 2015, Ole-André Rodlie <olear@fxarena.net>
 Copyright (c) 2015, FxArena DA <mail@fxarena.net>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 * Neither the name of FxArena DA nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


 Based on https://github.com/MrKepzie/openfx-io/blob/master/OIIO/OIIOText.cpp

 Written by Alexandre Gauthier <https://github.com/MrKepzie>

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 Neither the name of the {organization} nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 INRIA
 Domaine de Voluceau
 Rocquencourt - B.P. 105
 78153 Le Chesnay Cedex - France

*/

#include "Text.h"
#include "ofxsPositionInteract.h"
#include "ofxsMacros.h"
#include <Magick++.h>
#include <sstream>
#include <iostream>
#include <stdint.h>
#include <cmath>
#include <cstring>

#define CLAMP(value, min, max) (((value) >(max)) ? (max) : (((value) <(min)) ? (min) : (value)))

#define kPluginName "Text"
#define kPluginGrouping "Draw"

#define kPluginIdentifier "net.fxarena.openfx.Text"
#define kPluginVersionMajor 4
#define kPluginVersionMinor 0

#define kSupportsTiles 0
#define kSupportsMultiResolution 0
#define kSupportsRenderScale 1
#define kRenderThreadSafety eRenderInstanceSafe

#define kParamPosition "position"
#define kParamPositionLabel "Position"
#define kParamPositionHint "The position of the first character on the first line."

#define kParamInteractive "interactive"
#define kParamInteractiveLabel "Interactive"
#define kParamInteractiveHint "When checked the image will be rendered whenever moving the overlay interact instead of when releasing the mouse button."

#define kParamText "text"
#define kParamTextLabel "Text"
#define kParamTextHint "The text that will be drawn"

#define kParamFontSize "fontSize"
#define kParamFontSizeLabel "Size"
#define kParamFontSizeHint "The height of the characters to render in pixels"
#define kParamFontSizeDefault 64

#define kParamFontName "fontName"
#define kParamFontNameLabel "Font"
#define kParamFontNameHint "The name of the font to be used"
#define kParamFontNameDefault "Arial"
#define kParamFontNameAltDefault "DejaVu-Sans" // failsafe on Linux/BSD

#define kParamTextColor "textColor"
#define kParamTextColorLabel "Fill Color"
#define kParamTextColorHint "The fill color of the text to render"

#define kParamStrokeCheck "strokeCheck"
#define kParamStrokeCheckLabel "Outline"
#define kParamStrokeCheckHint "Enable or disable outline"
#define kParamStrokeCheckDefault false

#define kParamStrokeColor "strokeColor"
#define kParamStrokeColorLabel "Outline Color"
#define kParamStrokeColorHint "The stroke color of the text to render"

#define kParamStroke "stroke"
#define kParamStrokeLabel "Outline Width"
#define kParamStrokeHint "Adjust stroke width for outline"
#define kParamStrokeDefault 1

#define kParamFontOverride "customFont"
#define kParamFontOverrideLabel "Custom Font"
#define kParamFontOverrideHint "Override the font list. You can use font name or direct path"

#define kParamShadowCheck "shadow"
#define kParamShadowCheckLabel "Shadow"
#define kParamShadowCheckHint "Enable or disable drop shadow"
#define kParamShadowCheckDefault false

#define kParamShadowOpacity "shadowOpacity"
#define kParamShadowOpacityLabel "Shadow opacity"
#define kParamShadowOpacityHint "Adjust shadow opacity"
#define kParamShadowOpacityDefault 60

#define kParamShadowSigma "shadowOffset"
#define kParamShadowSigmaLabel "Shadow offset"
#define kParamShadowSigmaHint "Adjust shadow offset"
#define kParamShadowSigmaDefault 5

#define kParamInterlineSpacing "lineSpacing"
#define kParamInterlineSpacingLabel "Line spacing"
#define kParamInterlineSpacingHint "Spacing between lines"
#define kParamInterlineSpacingDefault 0

#define kParamInterwordSpacing "wordSpacing"
#define kParamInterwordSpacingLabel "Word spacing"
#define kParamInterwordSpacingHint "Spacing between words"
#define kParamInterwordSpacingDefault 0

#define kParamTextSpacing "letterSpacing"
#define kParamTextSpacingLabel "Letter spacing"
#define kParamTextSpacingHint "Spacing between letters"
#define kParamTextSpacingDefault 0

#define kParamPango "pango"
#define kParamPangoLabel "Pango markup"
#define kParamPangoHint "Enable/Disable Pango Markup Language.\n\n http://www.imagemagick.org/Usage/text/#pango"
#define kParamPangoDefault false

using namespace OFX;

class TextPlugin : public OFX::ImageEffect
{
public:
    TextPlugin(OfxImageEffectHandle handle);
    virtual ~TextPlugin();

    /* Override the render */
    virtual void render(const OFX::RenderArguments &args) OVERRIDE FINAL;

    /* override changedParam */
    virtual void changedParam(const OFX::InstanceChangedArgs &args, const std::string &paramName) OVERRIDE FINAL;

    // override the rod call
    virtual bool getRegionOfDefinition(const OFX::RegionOfDefinitionArguments &args, OfxRectD &rod) OVERRIDE FINAL;

private:
    // do not need to delete these, the ImageEffect is managing them for us
    OFX::Clip *dstClip_;
    OFX::Double2DParam *position_;
    OFX::StringParam *text_;
    OFX::IntParam *fontSize_;
    OFX::ChoiceParam *fontName_;
    OFX::RGBAParam *textColor_;
    OFX::RGBAParam *strokeColor_;
    OFX::BooleanParam *strokeEnabled_;
    OFX::DoubleParam *strokeWidth_;
    OFX::StringParam *fontOverride_;
    OFX::BooleanParam *shadowEnabled_;
    OFX::DoubleParam *shadowOpacity_;
    OFX::DoubleParam *shadowSigma_;
    OFX::DoubleParam *interlineSpacing_;
    OFX::DoubleParam *interwordSpacing_;
    OFX::DoubleParam *textSpacing_;
    OFX::BooleanParam *use_pango_;
    bool has_pango;
    bool has_fontconfig;
    bool has_freetype;
};

TextPlugin::TextPlugin(OfxImageEffectHandle handle)
: OFX::ImageEffect(handle)
, dstClip_(0)
{
    Magick::InitializeMagick(NULL);

    has_pango = false;
    has_fontconfig = false;
    has_freetype = false;

    std::string delegates = MagickCore::GetMagickDelegates();
    if (delegates.find("fontconfig") != std::string::npos)
        has_fontconfig = true;
    if (delegates.find("freetype") != std::string::npos)
        has_freetype = true;
    if (delegates.find("pango") != std::string::npos)
        has_pango = true;

    dstClip_ = fetchClip(kOfxImageEffectOutputClipName);
    assert(dstClip_ && dstClip_->getPixelComponents() == OFX::ePixelComponentRGBA);

    position_ = fetchDouble2DParam(kParamPosition);
    text_ = fetchStringParam(kParamText);
    fontSize_ = fetchIntParam(kParamFontSize);
    fontName_ = fetchChoiceParam(kParamFontName);
    textColor_ = fetchRGBAParam(kParamTextColor);
    strokeColor_ = fetchRGBAParam(kParamStrokeColor);
    strokeEnabled_ = fetchBooleanParam(kParamStrokeCheck);
    strokeWidth_ = fetchDoubleParam(kParamStroke);
    fontOverride_ = fetchStringParam(kParamFontOverride);
    shadowEnabled_ = fetchBooleanParam(kParamShadowCheck);
    shadowOpacity_ = fetchDoubleParam(kParamShadowOpacity);
    shadowSigma_ = fetchDoubleParam(kParamShadowSigma);
    interlineSpacing_ = fetchDoubleParam(kParamInterlineSpacing);
    interwordSpacing_ = fetchDoubleParam(kParamInterwordSpacing);
    textSpacing_ = fetchDoubleParam(kParamTextSpacing);
    use_pango_ = fetchBooleanParam(kParamPango);
    assert(position_ && text_ && fontSize_ && fontName_ && textColor_ && strokeColor_ && strokeEnabled_ && strokeWidth_ && fontOverride_ && shadowEnabled_ && shadowOpacity_ && shadowSigma_ && interlineSpacing_ && interwordSpacing_ && textSpacing_ && use_pango_);
}

TextPlugin::~TextPlugin()
{
}

/* Override the render */
void TextPlugin::render(const OFX::RenderArguments &args)
{
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    if (!dstClip_) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }
    assert(dstClip_);

    std::auto_ptr<OFX::Image> dstImg(dstClip_->fetchImage(args.time));
    if (!dstImg.get()) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    if (!has_fontconfig||!has_freetype) {
        setPersistentMessage(OFX::Message::eMessageError, "", "Fontconfig and/or Freetype missing");
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    if (dstImg->getRenderScale().x != args.renderScale.x ||
        dstImg->getRenderScale().y != args.renderScale.y ||
        dstImg->getField() != args.fieldToRender) {
        setPersistentMessage(OFX::Message::eMessageError, "", "OFX Host gave image with wrong scale or field properties");
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    OFX::BitDepthEnum dstBitDepth = dstImg->getPixelDepth();
    if (dstBitDepth != OFX::eBitDepthFloat) {
        OFX::throwSuiteStatusException(kOfxStatErrFormat);
        return;
    }

    OFX::PixelComponentEnum dstComponents  = dstImg->getPixelComponents();
    if (dstComponents != OFX::ePixelComponentRGBA) {
        OFX::throwSuiteStatusException(kOfxStatErrFormat);
        return;
    }

    // are we in the image bounds
    OfxRectI dstBounds = dstImg->getBounds();
    OfxRectI dstRod = dstImg->getRegionOfDefinition();
    if(args.renderWindow.x1 < dstBounds.x1 || args.renderWindow.x1 >= dstBounds.x2 || args.renderWindow.y1 < dstBounds.y1 || args.renderWindow.y1 >= dstBounds.y2 ||
       args.renderWindow.x2 <= dstBounds.x1 || args.renderWindow.x2 > dstBounds.x2 || args.renderWindow.y2 <= dstBounds.y1 || args.renderWindow.y2 > dstBounds.y2) {
        OFX::throwSuiteStatusException(kOfxStatErrValue);
        return;
    }

    // Get params
    double x, y, r, g, b, a, r_s, g_s, b_s, a_s, strokeWidth,shadowOpacity,shadowSigma,interlineSpacing,interwordSpacing,textSpacing;
    int fontSize, fontID;
    bool use_stroke = false;
    bool use_shadow = false;
    bool use_pango = false;
    std::string text, fontOverride, fontName;

    position_->getValueAtTime(args.time, x, y);
    text_->getValueAtTime(args.time, text);
    fontSize_->getValueAtTime(args.time, fontSize);
    fontName_->getValueAtTime(args.time, fontID);
    strokeEnabled_->getValueAtTime(args.time, use_stroke);
    strokeWidth_->getValueAtTime(args.time, strokeWidth);
    fontOverride_->getValueAtTime(args.time, fontOverride);
    textColor_->getValueAtTime(args.time, r, g, b, a);
    strokeColor_->getValueAtTime(args.time, r_s, g_s, b_s, a_s);
    shadowEnabled_->getValueAtTime(args.time, use_shadow);
    shadowOpacity_->getValueAtTime(args.time, shadowOpacity);
    shadowSigma_->getValueAtTime(args.time, shadowSigma);
    interlineSpacing_->getValueAtTime(args.time, interlineSpacing);
    interwordSpacing_->getValueAtTime(args.time, interwordSpacing);
    textSpacing_->getValueAtTime(args.time, textSpacing);
    use_pango_->getValueAtTime(args.time, use_pango);
    fontName_->getOption(fontID,fontName);

    // use custom font
    if (!fontOverride.empty())
        fontName=fontOverride;

    // Generate empty image
    int width = dstRod.x2-dstRod.x1;
    int height = dstRod.y2-dstRod.y1;
    Magick::Image image(Magick::Geometry(width,height),Magick::Color("rgba(0,0,0,0)"));

    // Set font size
    if (fontSize>0)
        image.fontPointsize(std::floor(fontSize * args.renderScale.x + 0.5));

    // Set stroke width
    if (use_stroke)
        image.strokeWidth(std::floor(strokeWidth * args.renderScale.x + 0.5));

    // Convert colors to int
    int rI = ((uint8_t)(255.0f *CLAMP(r, 0.0, 1.0)));
    int gI = ((uint8_t)(255.0f *CLAMP(g, 0.0, 1.0)));
    int bI = ((uint8_t)(255.0f *CLAMP(b, 0.0, 1.0)));
    /*int aI = ((uint8_t)(255.0f *CLAMP(a, 0.0, 1.0))); // enable on IM 6.9.1-3+
    a=aI;*/
    int r_sI = ((uint8_t)(255.0f *CLAMP(r_s, 0.0, 1.0)));
    int g_sI = ((uint8_t)(255.0f *CLAMP(g_s, 0.0, 1.0)));
    int b_sI = ((uint8_t)(255.0f *CLAMP(b_s, 0.0, 1.0)));
    /*int a_sI = ((uint8_t)(255.0f *CLAMP(a_s, 0.0, 1.0))); // enable on IM 6.9.1-3+
    a_s=a_sI;*/

    std::ostringstream rgba;
    rgba << "rgba(" << rI <<"," << gI << "," << bI << "," << a << ")";
    std::string textRGBA = rgba.str();

    std::ostringstream rgba_s;
    rgba_s << "rgba(" << r_sI <<"," << g_sI << "," << b_sI << "," << a_s << ")";
    std::string strokeRGBA = rgba_s.str();

    // Flip image
    image.flip();

    // Position x y
    double ytext = y*args.renderScale.y;
    double xtext = x*args.renderScale.x;
    int tmp_y = dstRod.y2 - dstBounds.y2;
    int tmp_height = dstBounds.y2 - dstBounds.y1;
    ytext = tmp_y + ((tmp_y+tmp_height-1) - ytext);

    // Setup draw
    std::list<Magick::Drawable> text_draw_list;
    text_draw_list.push_back(Magick::DrawableFont(fontName));
    text_draw_list.push_back(Magick::DrawableText(xtext, ytext, text));
    text_draw_list.push_back(Magick::DrawableFillColor(textRGBA));
    text_draw_list.push_back(Magick::DrawableTextInterlineSpacing(std::floor(interlineSpacing * args.renderScale.x + 0.5)));
    text_draw_list.push_back(Magick::DrawableTextInterwordSpacing(std::floor(interwordSpacing * args.renderScale.x + 0.5)));
    text_draw_list.push_back(Magick::DrawableTextKerning(std::floor(textSpacing * args.renderScale.x + 0.5)));
    if (use_stroke)
        text_draw_list.push_back(Magick::DrawableStrokeColor(strokeRGBA));

    // Draw
    if (has_pango && use_pango)
        image.read("pango:"+text);
    else
        image.draw(text_draw_list);

    // Shadow
    if (use_shadow) {
        Magick::Image dropShadow;
        dropShadow=image;
        dropShadow.backgroundColor("Black");
        dropShadow.virtualPixelMethod(Magick::TransparentVirtualPixelMethod);
        dropShadow.shadow(shadowOpacity,std::floor(shadowSigma * args.renderScale.x + 0.5),0,0);
        dropShadow.composite(image,0,0,Magick::OverCompositeOp);
        image=dropShadow;
    }

    // Flip image
    image.flip();

    // return image
    if (dstClip_ && dstClip_->isConnected()) {
        switch (dstBitDepth) {
        case eBitDepthUByte:
            if (image.depth()>8)
                image.depth(8);
            image.write(0,0,width,height,"RGBA",Magick::CharPixel,(float*)dstImg->getPixelData());
            break;
        case eBitDepthUShort:
            if (image.depth()>16)
                image.depth(16);
            image.write(0,0,width,height,"RGBA",Magick::ShortPixel,(float*)dstImg->getPixelData());
            break;
        case eBitDepthFloat:
            image.write(0,0,width,height,"RGBA",Magick::FloatPixel,(float*)dstImg->getPixelData());
            break;
        }
    }
}

void TextPlugin::changedParam(const OFX::InstanceChangedArgs &args, const std::string &/*paramName*/)
{
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    clearPersistentMessage();
}

bool TextPlugin::getRegionOfDefinition(const OFX::RegionOfDefinitionArguments &args, OfxRectD &rod)
{
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return false;
    }

    rod.x1 = rod.y1 = kOfxFlagInfiniteMin;
    rod.x2 = rod.y2 = kOfxFlagInfiniteMax;

    return true;
}

mDeclarePluginFactory(TextPluginFactory, {}, {});

namespace {
struct PositionInteractParam {
    static const char *name() { return kParamPosition; }
    static const char *interactiveName() { return kParamInteractive; }
};
}

/** @brief The basic describe function, passed a plugin descriptor */
void TextPluginFactory::describe(OFX::ImageEffectDescriptor &desc)
{
    // basic labels
    desc.setLabel(kPluginName);
    desc.setPluginGrouping(kPluginGrouping);
    std::string magickV = MagickCore::GetMagickVersion(NULL);
    std::string delegates = MagickCore::GetMagickDelegates();
    desc.setPluginDescription("Text generator for Natron.\n\nWritten by Ole-André Rodlie <olear@fxarena.net>\n\n Powered by "+magickV+"\n\nFeatures: "+delegates);

    // add the supported contexts
    desc.addSupportedContext(eContextGeneral);
    desc.addSupportedContext(eContextGenerator);

    // add supported pixel depths
    desc.addSupportedBitDepth(eBitDepthFloat);

    desc.setSupportsTiles(kSupportsTiles);
    desc.setSupportsMultiResolution(kSupportsMultiResolution);
    desc.setRenderThreadSafety(kRenderThreadSafety);

    desc.setOverlayInteractDescriptor(new PositionOverlayDescriptor<PositionInteractParam>);
}

/** @brief The describe in context function, passed a plugin descriptor and a context */
void TextPluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc, ContextEnum /*context*/)
{   
    // there has to be an input clip, even for generators
    ClipDescriptor* srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
    srcClip->addSupportedComponent(ePixelComponentRGBA);
    srcClip->setSupportsTiles(kSupportsTiles);
    srcClip->setOptional(true);

    // create the mandated output clip
    ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
    dstClip->addSupportedComponent(ePixelComponentRGBA);
    dstClip->setSupportsTiles(kSupportsTiles);

    // make some pages
    PageParamDescriptor *page = desc.definePageParam(kPluginName);

    bool hostHasNativeOverlayForPosition;
    {
        Double2DParamDescriptor* param = desc.defineDouble2DParam(kParamPosition);
        param->setLabel(kParamPositionLabel);
        param->setHint(kParamPositionHint);
        param->setDoubleType(eDoubleTypeXYAbsolute);
        param->setDefaultCoordinateSystem(eCoordinatesNormalised);
        param->setDefault(0.5, 0.5);
        param->setAnimates(true);
        hostHasNativeOverlayForPosition = param->getHostHasNativeOverlayHandle();
        if (hostHasNativeOverlayForPosition) {
            param->setUseHostOverlayHandle(true);
        }
        page->addChild(*param);
    }
    {
        BooleanParamDescriptor* param = desc.defineBooleanParam(kParamInteractive);
        param->setLabel(kParamInteractiveLabel);
        param->setHint(kParamInteractiveHint);
        param->setAnimates(false);
        page->addChild(*param);
        
        //Do not show this parameter if the host handles the interact
        if (hostHasNativeOverlayForPosition) {
            param->setIsSecret(true);
        }
    }
    {
        DoubleParamDescriptor *param = desc.defineDoubleParam(kParamTextSpacing);
        param->setLabel(kParamTextSpacingLabel);
        param->setHint(kParamTextSpacingHint);
        param->setRange(-1000, 1000);
        param->setDisplayRange(-100, 100);
        param->setDefault(kParamTextSpacingDefault);
        page->addChild(*param);
    }
    {
        DoubleParamDescriptor *param = desc.defineDoubleParam(kParamInterwordSpacing);
        param->setLabel(kParamInterwordSpacingLabel);
        param->setHint(kParamInterwordSpacingHint);
        param->setRange(-1000, 1000);
        param->setDisplayRange(-100, 100);
        param->setDefault(kParamInterwordSpacingDefault);
        page->addChild(*param);
    }
    {
        DoubleParamDescriptor *param = desc.defineDoubleParam(kParamInterlineSpacing);
        param->setLabel(kParamInterlineSpacingLabel);
        param->setHint(kParamInterlineSpacingHint);
        param->setRange(-1000, 1000);
        param->setDisplayRange(-100, 100);
        param->setDefault(kParamInterlineSpacingDefault);
        page->addChild(*param);
    }
    {
        BooleanParamDescriptor* param = desc.defineBooleanParam(kParamPango);
        param->setLabel(kParamPangoLabel);
        param->setHint(kParamPangoHint);
        param->setDefault(kParamPangoDefault);
        param->setAnimates(true);
        page->addChild(*param);

        std::string delegates = MagickCore::GetMagickDelegates();
        if (delegates.find("pango") != std::string::npos)
            param->setIsSecret(false);
        else
            param->setIsSecret(true);
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamText);
        param->setLabel(kParamTextLabel);
        param->setHint(kParamTextHint);
        param->setStringType(eStringTypeMultiLine);
        param->setAnimates(true);
        param->setDefault("Enter text");
        page->addChild(*param);
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamFontSize);
        param->setLabel(kParamFontSizeLabel);
        param->setHint(kParamFontSizeHint);
        param->setDefault(kParamFontSizeDefault);
        param->setAnimates(true);
        page->addChild(*param);
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamFontName);
        param->setLabel(kParamFontNameLabel);
        param->setHint(kParamFontNameHint);

        // Get all fonts
        int defaultFont = 0;
        int altFont = 0;
        char **fonts;
        std::size_t fontList;
        fonts=MagickCore::MagickQueryFonts("*",&fontList);
        for (size_t i=0;i<fontList;i++) {
          param->appendOption(fonts[i]);
          if (std::strcmp(fonts[i],kParamFontNameDefault)==0)
              defaultFont=i;
          if (std::strcmp(fonts[i],kParamFontNameAltDefault)==0)
              altFont=i;
        }

        for (size_t i = 0; i < fontList; i++)
            free(fonts[i]);

        if (defaultFont>0)
            param->setDefault(defaultFont);
        else if (defaultFont==0&&altFont>0)
            param->setDefault(altFont);

        param->setAnimates(true);
        page->addChild(*param);
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamFontOverride);
        param->setLabel(kParamFontOverrideLabel);
        param->setHint(kParamFontOverrideHint);
        param->setStringType(eStringTypeSingleLine);
        param->setAnimates(true);
        page->addChild(*param);
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamTextColor);
        param->setLabel(kParamTextColorLabel);
        param->setHint(kParamTextColorHint);
        param->setDefault(1., 1., 1., 1.);
        param->setAnimates(true);
        page->addChild(*param);
    }
    {
        BooleanParamDescriptor* param = desc.defineBooleanParam(kParamStrokeCheck);
        param->setLabel(kParamStrokeCheckLabel);
        param->setHint(kParamStrokeCheckHint);
        param->setEvaluateOnChange(true);
        param->setDefault(kParamStrokeCheckDefault);
        param->setAnimates(true);
        page->addChild(*param);
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamStrokeColor);
        param->setLabel(kParamStrokeColorLabel);
        param->setHint(kParamStrokeColorHint);
        param->setDefault(1., 1., 1., 1.);
        param->setAnimates(true);
        page->addChild(*param);
    }
    {
        DoubleParamDescriptor *param = desc.defineDoubleParam(kParamStroke);
        param->setLabel(kParamStrokeLabel);
        param->setHint(kParamStrokeHint);
        param->setRange(0, 50);
        param->setDisplayRange(0, 50);
        param->setDefault(kParamStrokeDefault);
        page->addChild(*param);
    }
    {
        BooleanParamDescriptor* param = desc.defineBooleanParam(kParamShadowCheck);
        param->setLabel(kParamShadowCheckLabel);
        param->setHint(kParamShadowCheckHint);
        param->setEvaluateOnChange(true);
        param->setDefault(kParamShadowCheckDefault);
        param->setAnimates(true);
        page->addChild(*param);
    }
    {
        DoubleParamDescriptor *param = desc.defineDoubleParam(kParamShadowOpacity);
        param->setLabel(kParamShadowOpacityLabel);
        param->setHint(kParamShadowOpacityHint);
        param->setRange(0, 100);
        param->setDisplayRange(0, 100);
        param->setDefault(kParamShadowOpacityDefault);
        page->addChild(*param);
    }
    {
        DoubleParamDescriptor *param = desc.defineDoubleParam(kParamShadowSigma);
        param->setLabel(kParamShadowSigmaLabel);
        param->setHint(kParamShadowSigmaHint);
        param->setRange(0, 100);
        param->setDisplayRange(0, 10);
        param->setDefault(kParamShadowSigmaDefault);
        page->addChild(*param);
    }
}

/** @brief The create instance function, the plugin must return an object derived from the \ref OFX::ImageEffect class */
ImageEffect* TextPluginFactory::createInstance(OfxImageEffectHandle handle, ContextEnum /*context*/)
{
    return new TextPlugin(handle);
}

void getTextPluginID(OFX::PluginFactoryArray &ids)
{
    static TextPluginFactory p(kPluginIdentifier, kPluginVersionMajor, kPluginVersionMinor);
    ids.push_back(&p);
}
