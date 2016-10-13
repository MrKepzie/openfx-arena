/*
 * This file is part of openfx-arena <https://github.com/olear/openfx-arena>,
 * Copyright (C) 2016 INRIA
 *
 * openfx-arena is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * openfx-arena is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with openfx-arena.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
*/

#include <pango/pangocairo.h>

#ifndef NOFC
#include <pango/pangofc-fontmap.h>
#include <fontconfig/fontconfig.h>
#else
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#endif

#include "ofxsMacros.h"
#include "ofxsImageEffect.h"
#include "ofxNatron.h"
#include "ofxsTransform3x3.h"
#include "ofxsTransformInteract.h"

#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <fstream>

#include "fx.h"

#define kPluginName "TextOFX"
#define kPluginGrouping "Draw"
#define kPluginIdentifier "net.fxarena.openfx.Text"
#define kPluginVersionMajor 6
#define kPluginVersionMinor 9

#define kSupportsTiles 0
#define kSupportsMultiResolution 0
#define kSupportsRenderScale 1
#define kRenderThreadSafety eRenderFullySafe

#define kParamText "text"
#define kParamTextLabel "Text"
#define kParamTextHint "The text that will be drawn."

#define kParamFontSize "size"
#define kParamFontSizeLabel "Font size"
#define kParamFontSizeHint "The height of the characters to render in pixels. Should not be used for animation, see the scale param."
#define kParamFontSizeDefault 64

#define kParamFontName "name"
#define kParamFontNameLabel "Font family"
#define kParamFontNameHint "The name of the font to be used."
#define kParamFontNameDefault "Arial"
#define kParamFontNameAltDefault "DejaVu Sans" // failsafe on Linux/BSD

#define kParamFont "font"
#define kParamFontLabel "Font"
#define kParamFontHint "Selected font."

#define kParamStyle "style"
#define kParamStyleLabel "Style"
#define kParamStyleHint "Font style."
#define kParamStyleDefault 0

#define kParamTextColor "color"
#define kParamTextColorLabel "Font color"
#define kParamTextColorHint "The fill color of the text to render."

#define kParamJustify "justify"
#define kParamJustifyLabel "Justify"
#define kParamJustifyHint "Text justify."
#define kParamJustifyDefault false

#define kParamWrap "wrap"
#define kParamWrapLabel "Wrap"
#define kParamWrapHint "Word wrap. Disabled if auto size or custom (transform) position is enabled."
#define kParamWrapDefault 0

#define kParamAlign "align"
#define kParamAlignLabel "Horizontal align"
#define kParamAlignHint "Horizontal text align. Disabled if custom position is enabled."
#define kParamAlignDefault 0

#define kParamVAlign "valign"
#define kParamVAlignLabel "Vertical align"
#define kParamVAlignHint "Vertical text align. Disabled if custom position is enabled."
#define kParamVAlignDefault 0

#define kParamMarkup "markup"
#define kParamMarkupLabel "Markup"
#define kParamMarkupHint "Pango Text Attribute Markup Language, https://developer.gnome.org/pango/stable/PangoMarkupFormat.html . Colors don't work if Circle/Arc effect is used, colors are also not supported on Windows at the moment."
#define kParamMarkupDefault false

#define kParamAutoSize "autoSize"
#define kParamAutoSizeLabel "Auto size"
#define kParamAutoSizeHint "Set canvas sized based on text. This will disable word wrap, custom canvas size and circle effect. Transform functions should also not be used in combination with this feature."
#define kParamAutoSizeDefault false

#define kParamStretch "stretch"
#define kParamStretchLabel "Stretch"
#define kParamStretchHint "Width of the font relative to other designs within a family."
#define kParamStretchDefault 4

#define kParamWeight "weight"
#define kParamWeightLabel "Weight"
#define kParamWeightHint "The weight field specifies how bold or light the font should be."
#define kParamWeightDefault 5

#define kParamStrokeColor "strokeColor"
#define kParamStrokeColorLabel "Stroke color"
#define kParamStrokeColorHint "The fill color of the stroke to render."

#define kParamStrokeWidth "strokeSize"
#define kParamStrokeWidthLabel "Stroke size"
#define kParamStrokeWidthHint "Stroke size."
#define kParamStrokeWidthDefault 0.0

#define kParamStrokeDash "strokeDash"
#define kParamStrokeDashLabel "Stroke dash length"
#define kParamStrokeDashHint "The length of the dashes."
#define kParamStrokeDashDefault 0

#define kParamStrokeDashPattern "strokeDashPattern"
#define kParamStrokeDashPatternLabel "Stroke dash pattern"
#define kParamStrokeDashPatternHint "An array specifying alternate lengths of on and off stroke portions."

#define kParamFontAA "antialiasing"
#define kParamFontAALabel "Antialiasing"
#define kParamFontAAHint "This specifies the type of antialiasing to do when rendering text."
#define kParamFontAADefault 0

#define kParamSubpixel "subpixel"
#define kParamSubpixelLabel "Subpixel"
#define kParamSubpixelHint " The subpixel order specifies the order of color elements within each pixel on the dets the antialiasing mode for the fontisplay device when rendering with an antialiasing mode."
#define kParamSubpixelDefault 0

#define kParamHintStyle "hintStyle"
#define kParamHintStyleLabel "Hint style"
#define kParamHintStyleHint "This controls whether to fit font outlines to the pixel grid, and if so, whether to optimize for fidelity or contrast."
#define kParamHintStyleDefault 0

#define kParamHintMetrics "hintMetrics"
#define kParamHintMetricsLabel "Hint metrics"
#define kParamHintMetricsHint "This controls whether metrics are quantized to integer values in device units."
#define kParamHintMetricsDefault 0

#define kParamLetterSpace "letterSpace"
#define kParamLetterSpaceLabel "Letter spacing"
#define kParamLetterSpaceHint "Spacing between letters. Disabled if markup is used."
#define kParamLetterSpaceDefault 0

#define kParamCircleRadius "circleRadius"
#define kParamCircleRadiusLabel "Circle radius"
#define kParamCircleRadiusHint "Circle radius. Effect only works if auto size is disabled."
#define kParamCircleRadiusDefault 0

#define kParamCircleWords "circleWords"
#define kParamCircleWordsLabel "Circle Words"
#define kParamCircleWordsHint "X times text in circle."
#define kParamCircleWordsDefault 10

#define kParamCanvas "canvas"
#define kParamCanvasLabel "Canvas size"
#define kParamCanvasHint "Set canvas size, default (0) is project format. Disabled if auto size is active."
#define kParamCanvasDefault 0

#define kParamArcRadius "arcRadius"
#define kParamArcRadiusLabel "Arc Radius"
#define kParamArcRadiusHint "Arc path radius (size of the path). The Arc effect is an experimental feature. Effect only works if auto size is disabled."
#define kParamArcRadiusDefault 100.0

#define kParamArcAngle "arcAngle"
#define kParamArcAngleLabel "Arc Angle"
#define kParamArcAngleHint "Arc Angle, set to 360 for a full circle. The Arc effect is an experimental feature. Effect only works if auto size is disabled."
#define kParamArcAngleDefault 0

#define kParamPositionMove "transform"
#define kParamPositionMoveLabel "Transform"
#define kParamPositionMoveHint "Use transform overlay for text position."
#define kParamPositionMoveDefault true

#define kParamTextFile "file"
#define kParamTextFileLabel "File"
#define kParamTextFileHint "Use text from filename."

#define kParamCenterInteract "centerInteract"
#define kParamCenterInteractLabel "Center Interact"
#define kParamCenterInteractHint "Center the text in the interact."
#define kParamCenterInteractDefault false

#define kParamFontOverride "custom"
#define kParamFontOverrideLabel "Custom font"
#define kParamFontOverrideHint "Add custom font."

using namespace OFX;
static bool gHostIsNatron = false;

bool stringCompare(const std::string & l, const std::string & r) {
    return (l==r);
}

#ifndef NOFC
std::list<std::string> _genFonts(OFX::ChoiceParam *fontName, OFX::StringParam *fontOverride, bool fontOverrideDir, FcConfig *fontConfig, bool properMenu, std::string fontNameDefault, std::string fontNameAltDefault)
{
    int defaultFont = 0;
    int altFont = 0;
    int fontIndex = 0;

    if (!fontConfig) {
        fontConfig = FcInitLoadConfigAndFonts();
    }

    if (fontOverride) {
        std::string fontCustom;
        fontOverride->getValue(fontCustom);
        if (!fontCustom.empty()) {
            const FcChar8 * fileCustom = (const FcChar8 *)fontCustom.c_str();
            if (!fontOverrideDir) {
                FcConfigAppFontAddFile(fontConfig,fileCustom);
            } else {
                FcConfigAppFontAddDir(fontConfig, fileCustom);
            }
        }
    }

    FcPattern *p = FcPatternCreate();
    FcObjectSet *os = FcObjectSetBuild (FC_FAMILY,NULL);
    FcFontSet *fs = FcFontList(fontConfig, p, os);
    std::list<std::string> fonts;
    for (int i=0; fs && i < fs->nfont; i++) {
        FcPattern *font = fs->fonts[i];
        FcChar8 *s;
        s = FcPatternFormat(font,(const FcChar8 *)"%{family[0]}");
        std::string fontName(reinterpret_cast<char*>(s));
        fonts.push_back(fontName);
        if (font) {
            FcPatternDestroy(font);
        }
    }

    fonts.sort();
    fonts.erase(unique(fonts.begin(), fonts.end(), stringCompare), fonts.end());

    if (fontName) {
        fontName->resetOptions();
    }
    std::list<std::string>::const_iterator font;
    for(font = fonts.begin(); font != fonts.end(); ++font) {
        std::string fontNameString = *font;
        std::string fontItem;
        if (properMenu) {
            fontItem=fontNameString[0];
            fontItem.append("/" + fontNameString);
        } else {
            fontItem=fontNameString;
        }

        if (fontName) {
            fontName->appendOption(fontItem);
        }
        if (std::strcmp(fontNameString.c_str(), fontNameDefault.c_str()) == 0) {
            defaultFont=fontIndex;
        }
        if (std::strcmp(fontNameString.c_str(), fontNameAltDefault.c_str()) == 0) {
            altFont=fontIndex;
        }

        fontIndex++;
    }

    if (fontName) {
        if (defaultFont > 0) {
            fontName->setDefault(defaultFont);
        } else if (altFont > 0) {
            fontName->setDefault(altFont);
        }
    }

    return fonts;
}
#else
std::list<std::string> _winFonts;
int CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
    if (FontType == TRUETYPE_FONTTYPE) {
        std::wstring wfont = lpelfe->elfFullName;
        std::string font(wfont.begin(), wfont.end());
        std::string prefix = "@";
        if (strncmp(font.c_str(), prefix.c_str(), strlen(prefix.c_str())) != 0) {
            _winFonts.push_back(font);
        }
        _winFonts.sort();
        _winFonts.erase(unique(_winFonts.begin(), _winFonts.end(), stringCompare), _winFonts.end());
    }
}
void _addWinFont(OFX::StringParam *fontParam)
{
    // NOTE! https://lists.cairographics.org/archives/cairo/2009-April/016984.html
    if (fontParam) {
        std::string font;
        fontParam->getValue(font);
        std::wstring wfont(font.begin(), font.end());
        AddFontResourceExW(wfont.c_str(), FR_PRIVATE, NULL);
    }
}
void _genWinFonts()
{
    _winFonts.clear();
    HDC hDC = GetDC(NULL);
    //LOGFONT lf = { 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, "" };
    LOGFONTW lf;
    EnumFontFamiliesExW(hDC, &lf, (FONTENUMPROCW)EnumFontFamiliesExProc, 0, 0);
    ReleaseDC(NULL, hDC);
}
void _popWinFonts(OFX::ChoiceParam *fontName, bool properMenu, std::string fontNameDefault, std::string fontNameAltDefault)
{
    int defaultFont = 0;
    int altFont = 0;
    int fontIndex = 0;

    if (fontName) {
        fontName->resetOptions();
    }
    std::list<std::string>::const_iterator font;
    for(font = _winFonts.begin(); font != _winFonts.end(); ++font) {
        std::string fontNameString = *font;
        std::string fontItem;
        if (properMenu) {
            fontItem=fontNameString[0];
            fontItem.append("/" + fontNameString);
        } else {
            fontItem=fontNameString;
        }

        if (fontName) {
            fontName->appendOption(fontItem);
        }
        if (std::strcmp(fontNameString.c_str(), fontNameDefault.c_str()) == 0) {
            defaultFont=fontIndex;
        }
        if (std::strcmp(fontNameString.c_str(), fontNameAltDefault.c_str()) == 0) {
            altFont=fontIndex;
        }

        fontIndex++;
    }

    if (fontName) {
        if (defaultFont > 0) {
            fontName->setDefault(defaultFont);
        } else if (altFont > 0) {
            fontName->setDefault(altFont);
        }
    }
}
#endif

class TextFXPlugin : public OFX::ImageEffect
{
public:
    TextFXPlugin(OfxImageEffectHandle handle);
    virtual ~TextFXPlugin();

    /* Override the render */
    virtual void render(const OFX::RenderArguments &args) OVERRIDE FINAL;

    /* override changedParam */
    virtual void changedParam(const OFX::InstanceChangedArgs &args, const std::string &paramName) OVERRIDE FINAL;

    // override the rod call
    virtual bool getRegionOfDefinition(const OFX::RegionOfDefinitionArguments &args, OfxRectD &rod) OVERRIDE FINAL;

    std::string textFromFile(std::string filename);
    void resetCenter(double time);
    void setFontDesc(int stretch, int weight, PangoFontDescription* desc);
    void setFontOpts(cairo_font_options_t* options, int hintStyle, int hintMetrics, int fontAA, int subpixel);

private:
    // do not need to delete these, the ImageEffect is managing them for us
    OFX::Clip *_dstClip;
    OFX::StringParam *_text;
    OFX::IntParam *_fontSize;
    OFX::RGBAParam *_textColor;
    OFX::StringParam *_font;
    OFX::BooleanParam *_justify;
    OFX::ChoiceParam *_wrap;
    OFX::ChoiceParam *_align;
    OFX::ChoiceParam *_valign;
    OFX::BooleanParam *_markup;
    OFX::ChoiceParam *_style;
    OFX::BooleanParam *auto_;
    OFX::ChoiceParam *stretch_;
    OFX::ChoiceParam *weight_;
    OFX::RGBAParam *strokeColor_;
    OFX::DoubleParam *strokeWidth_;
    OFX::IntParam *strokeDash_;
    OFX::Double3DParam *strokeDashPattern_;
    OFX::ChoiceParam *fontAA_;
    OFX::ChoiceParam *subpixel_;
    OFX::ChoiceParam *_hintStyle;
    OFX::ChoiceParam *_hintMetrics;
    OFX::DoubleParam *_circleRadius;
    OFX::IntParam *_circleWords;
    OFX::IntParam *_letterSpace;
    OFX::Int2DParam *_canvas;
    OFX::DoubleParam *_arcRadius;
    OFX::DoubleParam *_arcAngle;
    OFX::DoubleParam *_rotate;
    OFX::Double2DParam *_scale;
    OFX::Double2DParam *_position;
    OFX::BooleanParam *_move;
    OFX::StringParam *_txt;
    OFX::DoubleParam *_skewX;
    OFX::DoubleParam *_skewY;
    OFX::BooleanParam *_scaleUniform;
    OFX::BooleanParam *_centerInteract;
    OFX::ChoiceParam *_fontName;
    OFX::StringParam *_fontOverride;
    PangoLayout *_layout;
    cairo_surface_t *_surface;
#ifndef NOFC
    FcConfig* _fcConfig;
#endif
};

TextFXPlugin::TextFXPlugin(OfxImageEffectHandle handle)
: OFX::ImageEffect(handle)
, _dstClip(0)
, _text(0)
, _fontSize(0)
, _textColor(0)
, _font(0)
, _justify(0)
, _wrap(0)
, _align(0)
, _valign(0)
, _markup(0)
, _style(0)
, auto_(0)
, stretch_(0)
, weight_(0)
, strokeColor_(0)
, strokeWidth_(0)
, strokeDash_(0)
, strokeDashPattern_(0)
, fontAA_(0)
, subpixel_(0)
, _hintStyle(0)
, _hintMetrics(0)
, _circleRadius(0)
, _circleWords(0)
, _letterSpace(0)
, _canvas(0)
, _arcRadius(0)
, _arcAngle(0)
, _rotate(0)
, _scale(0)
, _position(0)
, _move(0)
, _txt(0)
, _skewX(0)
, _skewY(0)
, _scaleUniform(0)
, _centerInteract(0)
, _fontName(0)
, _fontOverride(0)
, _layout(0)
, _surface(0)
#ifndef NOFC
, _fcConfig(0)
#endif
{
    _dstClip = fetchClip(kOfxImageEffectOutputClipName);
    assert(_dstClip && _dstClip->getPixelComponents() == OFX::ePixelComponentRGBA);

    _text = fetchStringParam(kParamText);
    _fontSize = fetchIntParam(kParamFontSize);
    _fontName = fetchChoiceParam(kParamFontName);
    _textColor = fetchRGBAParam(kParamTextColor);
    _font = fetchStringParam(kParamFont);
    _justify = fetchBooleanParam(kParamJustify);
    _wrap = fetchChoiceParam(kParamWrap);
    _align = fetchChoiceParam(kParamAlign);
    _valign = fetchChoiceParam(kParamVAlign);
    _markup = fetchBooleanParam(kParamMarkup);
    _style = fetchChoiceParam(kParamStyle);
    auto_ = fetchBooleanParam(kParamAutoSize);
    stretch_ = fetchChoiceParam(kParamStretch);
    weight_ = fetchChoiceParam(kParamWeight);
    strokeColor_ = fetchRGBAParam(kParamStrokeColor);
    strokeWidth_ = fetchDoubleParam(kParamStrokeWidth);
    strokeDash_ = fetchIntParam(kParamStrokeDash);
    strokeDashPattern_ = fetchDouble3DParam(kParamStrokeDashPattern);
    fontAA_ = fetchChoiceParam(kParamFontAA);
    subpixel_ = fetchChoiceParam(kParamSubpixel);
    _hintStyle = fetchChoiceParam(kParamHintStyle);
    _hintMetrics = fetchChoiceParam(kParamHintMetrics);
    _circleRadius = fetchDoubleParam(kParamCircleRadius);
    _circleWords = fetchIntParam(kParamCircleWords);
    _letterSpace = fetchIntParam(kParamLetterSpace);
    _canvas = fetchInt2DParam(kParamCanvas);
    _arcRadius = fetchDoubleParam(kParamArcRadius);
    _arcAngle = fetchDoubleParam(kParamArcAngle);
    _rotate = fetchDoubleParam(kParamTransformRotateOld);
    _scale = fetchDouble2DParam(kParamTransformScaleOld);
    _position = fetchDouble2DParam(kParamTransformCenterOld);
    _move = fetchBooleanParam(kParamPositionMove);
    _txt = fetchStringParam(kParamTextFile);
    _skewX = fetchDoubleParam(kParamTransformSkewXOld);
    _skewY = fetchDoubleParam(kParamTransformSkewYOld);
    _scaleUniform = fetchBooleanParam(kParamTransformScaleUniformOld);
    _centerInteract = fetchBooleanParam(kParamCenterInteract);
    _fontOverride = fetchStringParam(kParamFontOverride);

    assert(_text && _fontSize && _fontName && _textColor && _font && _wrap
           && _justify && _align && _valign && _markup && _style && auto_ && stretch_ && weight_ && strokeColor_
           && strokeWidth_ && strokeDash_ && strokeDashPattern_ && fontAA_ && subpixel_ && _hintStyle
           && _hintMetrics && _circleRadius && _circleWords && _letterSpace && _canvas
           && _arcRadius && _arcAngle && _rotate && _scale && _position && _move && _txt
           && _skewX && _skewY && _scaleUniform && _centerInteract && _fontOverride);

#ifndef NOFC
    _fcConfig = FcInitLoadConfigAndFonts();
    _genFonts(_fontName, _fontOverride, false, _fcConfig, gHostIsNatron, kParamFontNameDefault, kParamFontNameAltDefault);
#else
    _addWinFont(_fontOverride);
    _genWinFonts();
    _popWinFonts(_fontName, gHostIsNatron, kParamFontNameDefault, kParamFontNameAltDefault);
#endif

    // Setup selected font
    std::string fontString, fontCombo;
    _font->getValue(fontString);
    int fontID;
    int fontCount = _fontName->getNOptions();
    _fontName->getValue(fontID);
    _fontName->getOption(fontID,fontCombo);
    if (!fontString.empty()) {
        if (std::strcmp(fontCombo.c_str(),fontString.c_str())!=0) {
            for(int x = 0; x < fontCount; x++) {
                std::string fontFound;
                _fontName->getOption(x,fontFound);
                if (!fontFound.empty()) {
                    if (std::strcmp(fontFound.c_str(),fontString.c_str())==0) {
                        _fontName->setValue(x);
                        break;
                    }
                }
            }
        }
    }
    else if (!fontCombo.empty()) {
        _font->setValue(fontCombo);
    }
}

TextFXPlugin::~TextFXPlugin()
{
    //g_object_unref(_layout);
    //cairo_surface_destroy(_surface);
}

void TextFXPlugin::resetCenter(double time) {
    if (!_dstClip) {
        return;
    }
    OfxRectD rod = _dstClip->getRegionOfDefinition(time);
    if ( (rod.x1 <= kOfxFlagInfiniteMin) || (kOfxFlagInfiniteMax <= rod.x2) ||
         ( rod.y1 <= kOfxFlagInfiniteMin) || ( kOfxFlagInfiniteMax <= rod.y2) ) {
        return;
    }
    OfxPointD newCenter;
    newCenter.x = (rod.x1 + rod.x2) / 2;
    newCenter.y = (rod.y1 + rod.y2) / 2;
    if (_position) {
        _position->setValue(newCenter.x, newCenter.y);
    }
}

std::string TextFXPlugin::textFromFile(std::string filename) {
    std::string result;
    if (!filename.empty()) {
        std::ifstream f;
        f.open(filename.c_str());
        std::ostringstream s;
        s << f.rdbuf();
        f.close();
        if (!s.str().empty()) {
            result = s.str();
        }
    }
    return result;
}

void TextFXPlugin::setFontDesc(int stretch, int weight, PangoFontDescription* desc)
{
    switch(stretch) {
    case 0:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_ULTRA_CONDENSED);
        break;
    case 1:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_EXTRA_CONDENSED);
        break;
    case 2:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_CONDENSED);
        break;
    case 3:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_SEMI_CONDENSED);
        break;
    case 4:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_NORMAL);
        break;
    case 5:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_SEMI_EXPANDED);
        break;
    case 6:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_EXPANDED);
        break;
    case 7:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_EXTRA_EXPANDED);
        break;
    case 8:
        pango_font_description_set_stretch(desc, PANGO_STRETCH_ULTRA_EXPANDED);
        break;
    }

    switch(weight) {
    case 0:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_THIN);
        break;
    case 1:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_ULTRALIGHT);
        break;
    case 2:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_LIGHT);
        break;
    case 3:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_SEMILIGHT);
        break;
    case 4:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_BOOK);
        break;
    case 5:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_NORMAL);
        break;
    case 6:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_MEDIUM);
        break;
    case 7:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_SEMIBOLD);
        break;
    case 8:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
        break;
    case 9:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_ULTRABOLD);
        break;
    case 10:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_HEAVY);
        break;
    case 11:
        pango_font_description_set_weight(desc, PANGO_WEIGHT_ULTRAHEAVY);
        break;
    }
}

void TextFXPlugin::setFontOpts(cairo_font_options_t* options, int hintStyle, int hintMetrics, int fontAA, int subpixel)
{
    switch(hintStyle) {
    case 0:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_DEFAULT);
        break;
    case 1:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_NONE);
        break;
    case 2:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_SLIGHT);
        break;
    case 3:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_MEDIUM);
        break;
    case 4:
        cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_FULL);
        break;
    }

    switch(hintMetrics) {
    case 0:
        cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_DEFAULT);
        break;
    case 1:
        cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_OFF);
        break;
    case 2:
        cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_ON);
        break;
    }

    switch(fontAA) {
    case 0:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_DEFAULT);
        break;
    case 1:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_NONE);
        break;
    case 2:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_GRAY);
        break;
    case 3:
        cairo_font_options_set_antialias(options, CAIRO_ANTIALIAS_SUBPIXEL);
        break;
    }

    switch(subpixel) {
    case 0:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_DEFAULT);
        break;
    case 1:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_RGB);
        break;
    case 2:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_BGR);
        break;
    case 3:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_VRGB);
        break;
    case 4:
        cairo_font_options_set_subpixel_order(options, CAIRO_SUBPIXEL_ORDER_VBGR);
        break;
    }

    pango_cairo_context_set_font_options(pango_layout_get_context(_layout), options);
}

/* Override the render */
void TextFXPlugin::render(const OFX::RenderArguments &args)
{
    // renderscale
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // dstclip
    if (!_dstClip) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }
    assert(_dstClip);

    // get dstclip
    std::auto_ptr<OFX::Image> dstImg(_dstClip->fetchImage(args.time));
    if (!dstImg.get()) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // renderscale
    if (dstImg->getRenderScale().x != args.renderScale.x ||
        dstImg->getRenderScale().y != args.renderScale.y ||
        dstImg->getField() != args.fieldToRender) {
        setPersistentMessage(OFX::Message::eMessageError, "", "OFX Host gave image with wrong scale or field properties");
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    // get bitdepth
    OFX::BitDepthEnum dstBitDepth = dstImg->getPixelDepth();
    if (dstBitDepth != OFX::eBitDepthFloat) {
        OFX::throwSuiteStatusException(kOfxStatErrFormat);
        return;
    }

    // get channels
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
    double x, y, r, g, b, a, s_r, s_g, s_b, s_a, strokeWidth, strokeDashX, strokeDashY, strokeDashZ, circleRadius, arcRadius, arcAngle, rotate, scaleX, scaleY, skewX, skewY;
    int fontSize, fontID, cwidth, cheight, wrap, align, valign, style, stretch, weight, strokeDash, fontAA, subpixel, hintStyle, hintMetrics, circleWords, letterSpace;
    std::string text, fontName, font, txt, fontOverride;
    bool justify = false;
    bool markup = false;
    bool autoSize = false;
    bool move = false;
    bool scaleUniform = false;
    bool centerInteract = false;

    _text->getValueAtTime(args.time, text);
    _fontSize->getValueAtTime(args.time, fontSize);
    _fontName->getValueAtTime(args.time, fontID);
    _textColor->getValueAtTime(args.time, r, g, b, a);
    _font->getValueAtTime(args.time, font);
    _fontName->getOption(fontID,fontName);
    _justify->getValueAtTime(args.time, justify);
    _wrap->getValueAtTime(args.time, wrap);
    _align->getValueAtTime(args.time, align);
    _valign->getValueAtTime(args.time, valign);
    _markup->getValueAtTime(args.time, markup);
    _style->getValueAtTime(args.time, style);
#ifndef NOFC
    auto_->getValueAtTime(args.time, autoSize);
#endif
    stretch_->getValueAtTime(args.time, stretch);
    weight_->getValueAtTime(args.time, weight);
    strokeColor_->getValueAtTime(args.time, s_r, s_g, s_b, s_a);
    strokeWidth_->getValueAtTime(args.time, strokeWidth);
    strokeDash_->getValueAtTime(args.time, strokeDash);
    strokeDashPattern_->getValueAtTime(args.time, strokeDashX, strokeDashY, strokeDashZ);
    fontAA_->getValueAtTime(args.time, fontAA);
    subpixel_->getValueAtTime(args.time, subpixel);
    _hintStyle->getValueAtTime(args.time, hintStyle);
    _hintMetrics->getValueAtTime(args.time, hintMetrics);
    _circleRadius->getValueAtTime(args.time, circleRadius);
    _circleWords->getValueAtTime(args.time, circleWords);
    _letterSpace->getValueAtTime(args.time, letterSpace);
    _canvas->getValueAtTime(args.time, cwidth, cheight);
    _arcRadius->getValueAtTime(args.time, arcRadius);
    _arcAngle->getValueAtTime(args.time, arcAngle);
    _rotate->getValueAtTime(args.time, rotate);
    _scale->getValueAtTime(args.time, scaleX, scaleY);
    _position->getValueAtTime(args.time, x, y);
    _move->getValueAtTime(args.time, move);
    _txt->getValueAtTime(args.time, txt);
    _skewX->getValueAtTime(args.time, skewX);
    _skewY->getValueAtTime(args.time, skewY);
    _scaleUniform->getValueAtTime(args.time, scaleUniform);
    _centerInteract->getValueAtTime(args.time, centerInteract);
    _fontOverride->getValueAtTime(args.time, fontOverride);

    double ytext = y*args.renderScale.y;
    double xtext = x*args.renderScale.x;
    int tmp_y = dstRod.y2 - dstBounds.y2;
    int tmp_height = dstBounds.y2 - dstBounds.y1;
    ytext = tmp_y + ((tmp_y+tmp_height-1) - ytext);

    if (!font.empty())
        fontName=font;
    else {
        setPersistentMessage(OFX::Message::eMessageError, "", "No fonts found/selected");
        OFX::throwSuiteStatusException(kOfxStatFailed);
    }

    if (gHostIsNatron) {
        fontName.erase(0,2);
    }

    if (!txt.empty()) {
        std::string txt_tmp = textFromFile(txt);
        if (!txt_tmp.empty()) {
            text = txt_tmp;
        }
    }

    std::ostringstream pangoFont;
    pangoFont << fontName;
    switch(style) {
    case 0:
        pangoFont << " " << "normal";
        break;
    case 1:
        pangoFont << " " << "bold";
        break;
    case 2:
        pangoFont << " " << "italic";
        break;
    }
    pangoFont << " " << std::floor(fontSize * args.renderScale.x + 0.5);

    int width = dstRod.x2-dstRod.x1;
    int height = dstRod.y2-dstRod.y1;

    cairo_t *cr;
    cairo_status_t status;
    PangoFontDescription *desc;
    PangoAttrList *alist;
    PangoFontMap* fontmap;

#ifndef NOFC
    fontmap = pango_cairo_font_map_get_default();
    if (pango_cairo_font_map_get_font_type((PangoCairoFontMap*)(fontmap)) != CAIRO_FONT_TYPE_FT) {
        fontmap = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
    }
    pango_fc_font_map_set_config((PangoFcFontMap*)fontmap, _fcConfig);
    pango_cairo_font_map_set_default((PangoCairoFontMap*)(fontmap));
#endif

    _surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create(_surface);

    _layout = pango_cairo_create_layout(cr);
    alist = pango_attr_list_new();

    cairo_font_options_t* options = cairo_font_options_create();
    setFontOpts(options, hintStyle, hintMetrics, fontAA, subpixel);

    if (markup) {
        pango_layout_set_markup(_layout, text.c_str(), -1);
    } else {
        pango_layout_set_text(_layout, text.c_str(), -1);
    }

    desc = pango_font_description_from_string(pangoFont.str().c_str());
    setFontDesc(stretch, weight, desc);

    pango_layout_set_font_description(_layout, desc);
    pango_font_description_free(desc);

    if (!autoSize && !move) {
        switch(wrap) {
        case 1:
            pango_layout_set_width(_layout, width * PANGO_SCALE);
            pango_layout_set_wrap(_layout, PANGO_WRAP_WORD);
            break;
        case 2:
            pango_layout_set_width(_layout, width * PANGO_SCALE);
            pango_layout_set_wrap(_layout, PANGO_WRAP_CHAR);
            break;
        case 3:
            pango_layout_set_width(_layout, width * PANGO_SCALE);
            pango_layout_set_wrap(_layout, PANGO_WRAP_WORD_CHAR);
            break;
        default:
            pango_layout_set_width(_layout, -1);
            break;
        }
    }

    if (!move) {
        switch(align) {
        case 0:
            pango_layout_set_alignment(_layout, PANGO_ALIGN_LEFT);
            break;
        case 1:
            pango_layout_set_alignment(_layout, PANGO_ALIGN_RIGHT);
            break;
        case 2:
            pango_layout_set_alignment(_layout, PANGO_ALIGN_CENTER);
            break;
        }

        if (valign != 0) {
            int text_width, text_height;
            pango_layout_get_pixel_size(_layout, &text_width, &text_height);
            switch (valign) {
            case 1:
                cairo_move_to(cr, 0, (height-text_height)/2);
                break;
            case 2:
                cairo_move_to(cr, 0, height-text_height);
                break;
            }
        }

        if (justify) {
            pango_layout_set_justify (_layout, true);
        }
    }

    if (letterSpace != 0) {
        pango_attr_list_insert(alist,pango_attr_letter_spacing_new(std::floor((letterSpace*PANGO_SCALE) * args.renderScale.x + 0.5)));
    }

    if (!markup) {
        pango_layout_set_attributes(_layout,alist);
    }

    if (!autoSize && !markup && circleRadius==0 && arcAngle==0 && strokeWidth==0 && move) {
        int moveX, moveY;
        if (centerInteract) {
            int text_width, text_height;
            pango_layout_get_pixel_size(_layout, &text_width, &text_height);
            moveX=xtext-(text_width/2);
            moveY=ytext-(text_height/2);
        } else {
            moveX=xtext;
            moveY=ytext;
        }
        cairo_move_to(cr, moveX, moveY);
    }

    if (scaleX!=1.0||scaleY!=1.0) {
        if (!autoSize && move) {
            cairo_translate(cr, xtext, ytext);
            if (scaleUniform) {
                cairo_scale(cr, scaleX, scaleX);
            } else {
                cairo_scale(cr, scaleX, scaleY);
            }
            cairo_translate(cr, -xtext, -ytext);
        }
    }

    if (skewX != 0.0 && !autoSize) {
        cairo_matrix_t matrixSkewX = {
            1.0, 0.0,
            -skewX, 1.0,
            0.0, 0.0
        };
        cairo_translate(cr, xtext, ytext);
        cairo_transform(cr, &matrixSkewX);
        cairo_translate(cr, -xtext, -ytext);
    }

    if (skewY !=0.0 && !autoSize) {
        cairo_matrix_t matrixSkewY = {
            1.0, -skewY,
            0.0, 1.0,
            0.0, 0.0
        };
        cairo_translate(cr, xtext, ytext);
        cairo_transform(cr, &matrixSkewY);
        cairo_translate(cr, -xtext, -ytext);
    }

    if (rotate !=0 && !autoSize) {
        double rotateX = width/2.0;
        double rotateY = height/2.0;
        if (move) {
            rotateX = xtext;
            rotateY = ytext;
        }
        cairo_translate(cr, rotateX, rotateY);
        cairo_rotate(cr, -rotate * (M_PI/180.0));
        cairo_translate(cr, -rotateX, -rotateY);
    }

    if (strokeWidth>0) {
        if (circleRadius==0) {
            if (strokeDash>0) {
                if (strokeDashX<0.1) {
                    strokeDashX=0.1;
                }
                if (strokeDashY<0) {
                    strokeDashY=0;
                }
                if (strokeDashZ<0) {
                    strokeDashZ=0;
                }
                double dash[] = {strokeDashX, strokeDashY, strokeDashZ};
                cairo_set_dash(cr, dash, strokeDash, 0);
            }

            cairo_new_path(cr);

            if (autoSize) {
                cairo_move_to(cr, std::floor((strokeWidth/2) * args.renderScale.x + 0.5), 0.0);
            } else if (move) {
                int moveX, moveY;
                if (centerInteract) {
                    int text_width, text_height;
                    pango_layout_get_pixel_size(_layout, &text_width, &text_height);
                    moveX=xtext-(text_width/2);
                    moveY=ytext-(text_height/2);
                } else {
                    moveX=xtext;
                    moveY=ytext;
                }
                cairo_move_to(cr, moveX, moveY);
            }

            pango_cairo_layout_path(cr, _layout);
            cairo_set_line_width(cr, std::floor(strokeWidth * args.renderScale.x + 0.5));
            //cairo_set_miter_limit(cr, );
            cairo_set_source_rgba(cr, s_r, s_g, s_b, s_a);
            cairo_stroke_preserve(cr);
            cairo_set_source_rgba(cr, r, g, b, a);
            cairo_fill(cr);
        }
    }
    else {
        if (circleRadius==0) {
            if (arcAngle>0) {
                double arcX = width/2.0;
                double arcY = height/2.0;
                if (move) {
                    arcX = xtext;
                    arcY = ytext;
                }
                cairo_arc(cr, arcX, arcY, std::floor(arcRadius * args.renderScale.x + 0.5), 0.0, arcAngle * (M_PI/180.0));
                cairo_path_t *path;
                cairo_save(cr);
                path = cairo_copy_path_flat(cr);
                cairo_new_path(cr);
                pango_cairo_layout_line_path(cr, pango_layout_get_line_readonly(_layout, 0)); //TODO if more than one line add support for that
                map_path_onto(cr, path);
                cairo_path_destroy(path);
                cairo_set_source_rgba(cr, r, g, b, a);
                cairo_fill(cr);
            }
            else {
                if (!autoSize && move) {
                    int moveX, moveY;
                    if (centerInteract) {
                        int text_width, text_height;
                        pango_layout_get_pixel_size(_layout, &text_width, &text_height);
                        moveX=xtext-(text_width/2);
                        moveY=ytext-(text_height/2);
                    } else {
                        moveX=xtext;
                        moveY=ytext;
                    }
                    cairo_move_to(cr, moveX, moveY);
                }
                cairo_set_source_rgba(cr, r, g, b, a);
                pango_cairo_update_layout(cr, _layout);
                pango_cairo_show_layout(cr, _layout);
            }
        }
    }

    if (circleRadius>0 && !autoSize) {
        double circleX = width/2.0;
        double circleY = height/2.0;
        if (move) {
            circleX = xtext;
            circleY = ytext;
        }
        cairo_translate(cr, circleX, circleY);
        for (int i = 0; i < circleWords; i++) {
            int rwidth, rheight;
            double angle = (360. * i) / circleWords;
            cairo_save(cr);
            cairo_set_source_rgba(cr, r, g, b, a);
            cairo_rotate(cr, angle * G_PI / 180.);
            pango_cairo_update_layout(cr, _layout);
            pango_layout_get_size(_layout, &rwidth, &rheight);
            cairo_move_to(cr, - ((double)rwidth / PANGO_SCALE) / 2, - std::floor(circleRadius * args.renderScale.x + 0.5));
            pango_cairo_layout_path(cr, _layout);
            cairo_fill(cr);
            cairo_restore (cr);
        }
    }

    status = cairo_status(cr);

    if (status) {
        setPersistentMessage(OFX::Message::eMessageError, "", "Render failed");
        OFX::throwSuiteStatusException(kOfxStatErrFormat);
    }

    cairo_surface_flush(_surface);

    unsigned char* cdata = cairo_image_surface_get_data(_surface);
    unsigned char* pixels = new unsigned char[width * height * 4];
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < 4; ++k)
                pixels[(i + j * width) * 4 + k] = cdata[(i + (height - 1 - j) * width) * 4 + k];
        }
    }

    float* pixelData = (float*)dstImg->getPixelData();
    int offset = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixelData[offset] = (float)pixels[offset + 2] / 255.f;
            pixelData[offset + 1] = (float)pixels[offset + 1] / 255.f;
            pixelData[offset + 2] = (float)pixels[offset] / 255.f;
            pixelData[offset + 3] = (float)pixels[offset + 3] / 255.f;
            offset += 4;
        }
    }

    pango_attr_list_unref(alist);
    cairo_font_options_destroy(options);
    cairo_destroy(cr);
    cdata = NULL;
    pixelData = NULL;
    delete[] pixels;
}

void TextFXPlugin::changedParam(const OFX::InstanceChangedArgs &args, const std::string &paramName)
{
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return;
    }

    if (paramName == kParamTransformResetCenterOld) {
        resetCenter(args.time);
    } else if (paramName == kParamFontName) {
        std::string font;
        int fontID;
        _fontName->getValueAtTime(args.time, fontID);
        _fontName->getOption(fontID,font);
        _font->setValue(font);
    } else if (paramName == kParamFontOverride) {
        int selectedFontIndex  = 0;
        _fontName->getValueAtTime(args.time, selectedFontIndex);
        std::string selectedFontName;
        _fontName->getOption(selectedFontIndex, selectedFontName);
        if (gHostIsNatron) {
            selectedFontName.erase(0,2);
        }
        if (selectedFontName.empty()) {
            selectedFontName = kParamFontNameDefault;
        }

#ifndef NOFC
        _genFonts(_fontName, _fontOverride, false, _fcConfig, gHostIsNatron, selectedFontName, kParamFontNameAltDefault);
#else
        _addWinFont(_fontOverride);
        _genWinFonts();
        _popWinFonts(_fontName, gHostIsNatron, selectedFontName, kParamFontNameAltDefault);
#endif

    }

    clearPersistentMessage();
}

bool TextFXPlugin::getRegionOfDefinition(const OFX::RegionOfDefinitionArguments &args, OfxRectD &rod)
{
    if (!kSupportsRenderScale && (args.renderScale.x != 1. || args.renderScale.y != 1.)) {
        OFX::throwSuiteStatusException(kOfxStatFailed);
        return false;
    }

    int width,height;
    _canvas->getValue(width, height);

#ifndef NOFC
    bool autoSize = false;
    auto_->getValue(autoSize);

    if (autoSize) {
        int fontSize, fontID, style, stretch, weight, letterSpace;
        double strokeWidth;
        std::string text, fontName, font, txt;
        bool markup = false;

        _text->getValueAtTime(args.time, text);
        _fontSize->getValueAtTime(args.time, fontSize);
        _fontName->getValueAtTime(args.time, fontID);
        _font->getValueAtTime(args.time, font);
        _fontName->getOption(fontID,fontName);
        _style->getValueAtTime(args.time, style);
        _markup->getValueAtTime(args.time, markup);
        stretch_->getValueAtTime(args.time, stretch);
        weight_->getValueAtTime(args.time, weight);
        strokeWidth_->getValueAtTime(args.time, strokeWidth);
        _letterSpace->getValueAtTime(args.time, letterSpace);
        _txt->getValueAtTime(args.time, txt);

        if (!txt.empty()) {
            std::string txt_tmp = textFromFile(txt);
            if (!txt_tmp.empty()) {
                text = txt_tmp;
            }
        }

        if (!font.empty()) {
            fontName=font;
            if (gHostIsNatron)
                fontName.erase(0,2);

            std::ostringstream pangoFont;
            pangoFont << fontName;
            switch(style) {
            case 0:
                pangoFont << " " << "normal";
                break;
            case 1:
                pangoFont << " " << "bold";
                break;
            case 2:
                pangoFont << " " << "italic";
                break;
            }
            pangoFont << " " << fontSize;

            width = rod.x2-rod.x1;
            height = rod.y2-rod.y1;

            cairo_t *cr;
            PangoFontDescription *desc;
            PangoAttrList *alist;
            PangoFontMap* fontmap;

            fontmap = pango_cairo_font_map_get_default();
            if (pango_cairo_font_map_get_font_type((PangoCairoFontMap*)(fontmap)) != CAIRO_FONT_TYPE_FT) {
                fontmap = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
            }
            pango_fc_font_map_set_config((PangoFcFontMap*)fontmap, _fcConfig);
            pango_cairo_font_map_set_default((PangoCairoFontMap*)(fontmap));

            _surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
            cr = cairo_create (_surface);

            _layout = pango_cairo_create_layout(cr);
            alist = pango_attr_list_new();

            if (markup) {
                pango_layout_set_markup(_layout, text.c_str(), -1);
            } else {
                pango_layout_set_text(_layout, text.c_str(), -1);
            }

            desc = pango_font_description_from_string(pangoFont.str().c_str());
            setFontDesc(stretch, weight, desc);

            pango_layout_set_font_description(_layout, desc);
            pango_font_description_free(desc);

            if (letterSpace != 0) {
                pango_attr_list_insert(alist,pango_attr_letter_spacing_new(letterSpace*PANGO_SCALE));
            }

            if (!markup) {
                pango_layout_set_attributes(_layout,alist);
            }

            pango_layout_get_pixel_size(_layout, &width, &height);

            /// WIP
            if (strokeWidth>0) {
                width = width+(strokeWidth*2);
                height = height+(strokeWidth/2);
            }

            pango_attr_list_unref(alist);
            cairo_destroy(cr);
        }
    }
#endif

    if (width>0 && height>0) {
        rod.x1 = rod.y1 = 0;
        rod.x2 = width;
        rod.y2 = height;
    }
    else {
        //rod.x1 = rod.y1 = kOfxFlagInfiniteMin;
        //rod.x2 = rod.y2 = kOfxFlagInfiniteMax;
        return false;
    }

    return true;
}

mDeclarePluginFactory(TextFXPluginFactory, {}, {});

/** @brief The basic describe function, passed a plugin descriptor */
void TextFXPluginFactory::describe(OFX::ImageEffectDescriptor &desc)
{
    // basic labels
    desc.setLabel(kPluginName);
    desc.setPluginGrouping(kPluginGrouping);
    desc.setPluginDescription("Advanced text generator node using Pango and Cairo.");

    // add the supported contexts
    desc.addSupportedContext(eContextGenerator);

    // add supported pixel depths
    desc.addSupportedBitDepth(eBitDepthFloat);

    // add other
    desc.setSupportsTiles(kSupportsTiles);
    desc.setSupportsMultiResolution(kSupportsMultiResolution);
    desc.setRenderThreadSafety(kRenderThreadSafety);

    Transform3x3Describe(desc, true);
    desc.setOverlayInteractDescriptor(new TransformOverlayDescriptorOldParams);
}

/** @brief The describe in context function, passed a plugin descriptor and a context */
void TextFXPluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc, ContextEnum /*context*/)
{
    // natron?
    gHostIsNatron = (OFX::getImageEffectHostDescription()->isNatron);

    // there has to be an input clip, even for generators
    ClipDescriptor* srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
    srcClip->setOptional(true);

    // create the mandated output clip
    ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
    dstClip->addSupportedComponent(ePixelComponentRGBA);
    dstClip->setSupportsTiles(kSupportsTiles);

    // make some pages
    PageParamDescriptor *page = desc.definePageParam(kPluginName);
    ofxsTransformDescribeParams(desc, page, NULL, /*isOpen=*/ true, /*oldParams=*/ true, /*noTranslate=*/ true);
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamPositionMove);
        param->setLabel(kParamPositionMoveLabel);
        param->setHint(kParamPositionMoveHint);
        param->setDefault(kParamPositionMoveDefault);
        param->setAnimates(false);
        param->setLayoutHint(eLayoutHintNoNewLine, 1);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamAutoSize);
        param->setLabel(kParamAutoSizeLabel);
        param->setHint(kParamAutoSizeHint);
        param->setAnimates(false);
        param->setDefault(kParamAutoSizeDefault);
        param->setLayoutHint(eLayoutHintNoNewLine, 1);
#ifdef NOFC
        param->setIsSecret(true);
#endif
        if (page) {
            page->addChild(*param);
        }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamCenterInteract);
        param->setLabel(kParamCenterInteractLabel);
        param->setHint(kParamCenterInteractHint);
        param->setDefault(kParamCenterInteractDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        Int2DParamDescriptor* param = desc.defineInt2DParam(kParamCanvas);
        param->setLabel(kParamCanvasLabel);
        param->setHint(kParamCanvasHint);
        param->setRange(0, 0, 10000, 10000);
        param->setDisplayRange(0, 0, 4000, 4000);
        param->setDefault(kParamCanvasDefault, kParamCanvasDefault);
        param->setAnimates(false);
        param->setLayoutHint(OFX::eLayoutHintDivider);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamMarkup);
        param->setLabel(kParamMarkupLabel);
        param->setHint(kParamMarkupHint);
        param->setDefault(kParamMarkupDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamTextFile);
        param->setLabel(kParamTextFileLabel);
        param->setHint(kParamTextFileHint);
        param->setStringType(eStringTypeFilePath);
        param->setFilePathExists(true);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamText);
        param->setLabel(kParamTextLabel);
        param->setHint(kParamTextHint);
        param->setStringType(eStringTypeMultiLine);
        param->setAnimates(true);
        param->setDefault("Enter text");
        if (page) {
            page->addChild(*param);
        }
    }
    {
        BooleanParamDescriptor *param = desc.defineBooleanParam(kParamJustify);
        param->setLabel(kParamJustifyLabel);
        param->setHint(kParamJustifyHint);
        param->setDefault(kParamJustifyDefault);
        param->setLayoutHint(eLayoutHintNoNewLine, 1);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamWrap);
        param->setLabel(kParamWrapLabel);
        param->setHint(kParamWrapHint);
        param->appendOption("None");
        param->appendOption("Word");
        param->appendOption("Char");
        param->appendOption("Word-Char");
        param->setDefault(kParamWrapDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamAlign);
        param->setLabel(kParamAlignLabel);
        param->setHint(kParamAlignHint);
        param->appendOption("Left");
        param->appendOption("Right");
        param->appendOption("Center");
        param->setDefault(kParamAlignDefault);
        param->setAnimates(false);
        param->setLayoutHint(eLayoutHintNoNewLine, 1);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamVAlign);
        param->setLabel(kParamVAlignLabel);
        param->setHint(kParamVAlignHint);
        param->appendOption("Top");
        param->appendOption("Center");
        param->appendOption("Bottom");
        param->setDefault(kParamVAlignDefault);
        param->setAnimates(false);
        param->setLayoutHint(OFX::eLayoutHintDivider);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamFontName);
        param->setLabel(kParamFontNameLabel);
        param->setHint(kParamFontNameHint);
        param->setAnimates(false);
        if (gHostIsNatron) {
            param->setCascading(OFX::getImageEffectHostDescription()->supportsCascadingChoices);
        }
        std::list<std::string>::const_iterator font;
        std::list<std::string> fonts;

#ifndef NOFC
        fonts = _genFonts(NULL, NULL, false, NULL, false, kParamFontNameDefault, kParamFontNameAltDefault);
#else
        _genWinFonts();
        fonts = _winFonts;
#endif

        int defaultFont = 0;
        int altFont = 0;
        int fontIndex = 0;
        for(font = fonts.begin(); font != fonts.end(); ++font) {
            std::string fontName = *font;
            if (!fontName.empty()) {
                std::string fontItem;
                if (gHostIsNatron) {
                    fontItem=fontName[0];
                    fontItem.append("/" + fontName);
                } else {
                    fontItem=fontName;
                }
                param->appendOption(fontItem);
                if (std::strcmp(fontName.c_str(), kParamFontNameDefault) == 0) {
                    defaultFont=fontIndex;
                }
                if (std::strcmp(fontName.c_str(), kParamFontNameAltDefault) == 0) {
                    altFont=fontIndex;
                }
            }
            fontIndex++;
        }
        if (fonts.empty()) {
            param->appendOption("N/A");
        }
        if (defaultFont > 0) {
            param->setDefault(defaultFont);
        } else if (altFont > 0) {
            param->setDefault(altFont);
        }
        if (page) {
            page->addChild(*param);
        }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamFontOverride);
        param->setLabel(kParamFontOverrideLabel);
        param->setHint(kParamFontOverrideHint);
        param->setStringType(eStringTypeFilePath);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        StringParamDescriptor* param = desc.defineStringParam(kParamFont);
        param->setLabel(kParamFontLabel);
        param->setHint(kParamFontHint);
        param->setStringType(eStringTypeSingleLine);
        param->setAnimates(false);

        #ifdef DEBUG
        param->setIsSecret(false);
        #else
        param->setIsSecret(true);
        #endif

        if (page) {
            page->addChild(*param);
        }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamFontSize);
        param->setLabel(kParamFontSizeLabel);
        param->setHint(kParamFontSizeHint);
        param->setRange(1, 10000);
        param->setDisplayRange(1, 500);
        param->setDefault(kParamFontSizeDefault);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamTextColor);
        param->setLabel(kParamTextColorLabel);
        param->setHint(kParamTextColorHint);
        param->setDefault(1., 1., 1., 1.);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamLetterSpace);
        param->setLabel(kParamLetterSpaceLabel);
        param->setHint(kParamLetterSpaceHint);
        param->setRange(0, 10000);
        param->setDisplayRange(0, 500);
        param->setDefault(kParamLetterSpaceDefault);
        param->setLayoutHint(OFX::eLayoutHintDivider);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamHintStyle);
        param->setLabel(kParamHintStyleLabel);
        param->setHint(kParamHintStyleHint);
        param->appendOption("Default");
        param->appendOption("None");
        param->appendOption("Slight");
        param->appendOption("Medium");
        param->appendOption("Full");
        param->setDefault(kParamHintStyleDefault);
        param->setLayoutHint(eLayoutHintNoNewLine, 1);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamHintMetrics);
        param->setLabel(kParamHintMetricsLabel);
        param->setHint(kParamHintMetricsHint);
        param->appendOption("Default");
        param->appendOption("Off");
        param->appendOption("On");
        param->setDefault(kParamHintMetricsDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamFontAA);
        param->setLabel(kParamFontAALabel);
        param->setHint(kParamFontAAHint);
        param->appendOption("Default");
        param->appendOption("None");
        param->appendOption("Gray");
        param->appendOption("Subpixel");
        param->setDefault(kParamFontAADefault);
        param->setLayoutHint(eLayoutHintNoNewLine, 1);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamSubpixel);
        param->setLabel(kParamSubpixelLabel);
        param->setHint(kParamSubpixelHint);
        param->appendOption("Default");
        param->appendOption("RGB");
        param->appendOption("BGR");
        param->appendOption("VRGB");
        param->appendOption("VBGR");
        param->setDefault(kParamSubpixelDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamStyle);
        param->setLabel(kParamStyleLabel);
        param->setHint(kParamStyleHint);
        param->appendOption("Normal");
        param->appendOption("Bold");
        param->appendOption("Italic");
        param->setDefault(kParamStyleDefault);
        param->setLayoutHint(eLayoutHintNoNewLine, 1);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamWeight);
        param->setLabel(kParamWeightLabel);
        param->setHint(kParamWeightHint);
        param->appendOption("Thin");
        param->appendOption("Ultra light");
        param->appendOption("Light");
        param->appendOption("Semi light");
        param->appendOption("Book");
        param->appendOption("Normal");
        param->appendOption("Medium");
        param->appendOption("Semi bold");
        param->appendOption("Bold");
        param->appendOption("Ultra bold");
        param->appendOption("Heavy");
        param->appendOption("Ultra heavy");
        param->setDefault(kParamWeightDefault);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam(kParamStretch);
        param->setLabel(kParamStretchLabel);
        param->setHint(kParamStretchHint);
        param->appendOption("Ultra condensed");
        param->appendOption("Extra condensed");
        param->appendOption("Condensed");
        param->appendOption("Semi condensed");
        param->appendOption("Normal");
        param->appendOption("Semi expanded");
        param->appendOption("Expanded");
        param->appendOption("Extra expanded");
        param->appendOption("Ultra expanded");
        param->setDefault(kParamStretchDefault);
        param->setLayoutHint(OFX::eLayoutHintDivider);
        param->setAnimates(false);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamStrokeWidth);
        param->setLabel(kParamStrokeWidthLabel);
        param->setHint(kParamStrokeWidthHint);
        param->setRange(0, 500);
        param->setDisplayRange(0, 100);
        param->setDefault(kParamStrokeWidthDefault);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        RGBAParamDescriptor* param = desc.defineRGBAParam(kParamStrokeColor);
        param->setLabel(kParamStrokeColorLabel);
        param->setHint(kParamStrokeColorHint);
        param->setDefault(1., 0., 0., 1.);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamStrokeDash);
        param->setLabel(kParamStrokeDashLabel);
        param->setHint(kParamStrokeDashHint);
        param->setRange(0, 100);
        param->setDisplayRange(0, 10);
        param->setDefault(kParamStrokeDashDefault);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        Double3DParamDescriptor* param = desc.defineDouble3DParam(kParamStrokeDashPattern);
        param->setLabel(kParamStrokeDashPatternLabel);
        param->setHint(kParamStrokeDashPatternHint);
        param->setDefault(1.0, 0.0, 0.0);
        param->setAnimates(true);
        param->setLayoutHint(OFX::eLayoutHintDivider);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamCircleRadius);
        param->setLabel(kParamCircleRadiusLabel);
        param->setHint(kParamCircleRadiusHint);
        param->setRange(0, 10000);
        param->setDisplayRange(0, 1000);
        param->setDefault(kParamCircleRadiusDefault);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        IntParamDescriptor* param = desc.defineIntParam(kParamCircleWords);
        param->setLabel(kParamCircleWordsLabel);
        param->setHint(kParamCircleWordsHint);
        param->setRange(1, 1000);
        param->setDisplayRange(1, 100);
        param->setDefault(kParamCircleWordsDefault);
        param->setAnimates(true);
        param->setLayoutHint(OFX::eLayoutHintDivider);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamArcRadius);
        param->setLabel(kParamArcRadiusLabel);
        param->setHint(kParamArcRadiusHint);
        param->setRange(0, 10000);
        param->setDisplayRange(0, 1000);
        param->setDefault(kParamArcRadiusDefault);
        param->setAnimates(true);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        DoubleParamDescriptor* param = desc.defineDoubleParam(kParamArcAngle);
        param->setLabel(kParamArcAngleLabel);
        param->setHint(kParamArcAngleHint);
        param->setRange(0, 360);
        param->setDisplayRange(0, 360);
        param->setDefault(kParamArcAngleDefault);
        param->setAnimates(true);
        param->setLayoutHint(OFX::eLayoutHintDivider);
        if (page) {
            page->addChild(*param);
        }
    }
}

/** @brief The create instance function, the plugin must return an object derived from the \ref OFX::ImageEffect class */
ImageEffect* TextFXPluginFactory::createInstance(OfxImageEffectHandle handle, ContextEnum /*context*/)
{
    return new TextFXPlugin(handle);
}

static TextFXPluginFactory p(kPluginIdentifier, kPluginVersionMajor, kPluginVersionMinor);
mRegisterPluginFactoryInstance(p)
