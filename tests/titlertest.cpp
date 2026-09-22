/*
    SPDX-FileCopyrightText: 2022 Eric Jiang
    SPDX-FileCopyrightText: 2022 Jean-Baptiste Mardelle <jb@kdenlive.org>
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "test_utils.hpp"
// test specific headers
#include "titler/graphicsscenerectmove.h"
#include "titler/titledocument.h"
#include "titler/richtextspacing.h"
#include "titler/richtextgradient.h"

TEST_CASE("Title text left alignment", "[Titler]")
{
    QScopedPointer<MyTextItem> txt(new MyTextItem("Hello, world!", nullptr));
    txt->setAlignment(Qt::AlignLeft);
    QRectF origBB = txt->boundingRect();
    qreal origX = txt->x();
    txt->document()->setPlainText("Hello, longer string!");
    QRectF newBB = txt->boundingRect();
    qreal newX = txt->x();

    // make sure the left and right edges of the title are still aligned properly
    CHECK(newBB.width() > 0);
    CHECK(origBB.topRight().x() < newBB.topRight().x());
    CHECK(newX == Approx(origX));
}

TEST_CASE("Title text right alignment", "[Titler]")
{
    QScopedPointer<MyTextItem> txt(new MyTextItem("Hello, world!", nullptr));
    txt->setAlignment(Qt::AlignRight);
    QRectF origBB = txt->boundingRect();
    // origX is the left edge of the txt object
    qreal origX = txt->x();
    // origRightX is the right edge of the txt object
    qreal origRightX = origBB.width() + origX;
    txt->document()->setPlainText("Hello, longer string!");
    QRectF newBB = txt->boundingRect();
    qreal newX = txt->x();
    qreal newRightX = newBB.width() + newX;

    CHECK(newBB.width() > 0);
    CHECK(origRightX == Approx(newRightX));
    CHECK(origX > newX);
}

TEST_CASE("Title text center alignment", "[Titler]")
{
    QScopedPointer<MyTextItem> txt(new MyTextItem("short", nullptr));
    txt->setAlignment(Qt::AlignHCenter);
    QRectF origBB = txt->boundingRect();
    qreal origX = txt->x();
    qreal origRightX = origBB.width() + origX;
    qreal origCenter = (origX + origRightX) / 2;
    txt->document()->setPlainText("longer string");
    QRectF newBB = txt->boundingRect();
    qreal newX = txt->x();
    qreal newRightX = newBB.width() + newX;
    qreal newCenter = (newX + newRightX) / 2;

    CHECK(origCenter == Approx(newCenter));
    CHECK(newRightX > origRightX);
    CHECK(newX < origX);
}

// RichText: regression tests exercise the real MyTextItem callbacks.
#include "titler/richtextformat.h"
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>

namespace {
void richSelect(MyTextItem *item, int anchor, int position)
{
    QTextCursor cursor(item->document());
    cursor.setPosition(anchor);
    cursor.setPosition(position, QTextCursor::KeepAnchor);
    item->setTextCursor(cursor);
}
QTextCharFormat richFormat(MyTextItem *item, int position)
{
    QTextCursor cursor(item->document());
    cursor.setPosition(position);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    return cursor.charFormat();
}
}

TEST_CASE("Rich text color keeps fonts and selection", "[Titler][RichText]")
{
    MyTextItem item(QStringLiteral("Hello amazing world"), nullptr);
    item.setTextInteractionFlags(Qt::TextEditorInteraction);
    QTextCharFormat base;
    base.setFontFamilies(QStringList{QStringLiteral("serif")});
    base.setProperty(QTextFormat::FontPixelSize, 28);
    base.setFontWeight(QFont::Normal);
    base.setForeground(QBrush(Qt::white));
    richSelect(&item, 0, 19);
    TitlerRichText::apply(&item, base);
    const auto outside = richFormat(&item, 0);

    richSelect(&item, 13, 6); // Backward selections must survive too.
    QTextCharFormat family, size, bold, italic, underline, spacing, red;
    family.setFontFamilies(QStringList{QStringLiteral("monospace")});
    size.setProperty(QTextFormat::FontPixelSize, 48);
    bold.setFontWeight(QFont::Bold);
    italic.setFontItalic(true);
    underline.setFontUnderline(true);
    spacing.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    spacing.setFontLetterSpacing(3);
    red.setForeground(QBrush(Qt::red));
    for (const auto &delta : {family, size, bold, italic, underline, spacing, red}) {
        TitlerRichText::apply(&item, delta);
        REQUIRE(item.textCursor().anchor() == 13);
        REQUIRE(item.textCursor().position() == 6);
        REQUIRE(richFormat(&item, 0) == outside);
        REQUIRE(richFormat(&item, 14) == outside);
    }
    const auto styled = richFormat(&item, 8);
    REQUIRE(styled.fontFamilies().toStringList() == QStringList{QStringLiteral("monospace")});
    REQUIRE(styled.font().pixelSize() == 48);
    REQUIRE(styled.fontWeight() == QFont::Bold);
    REQUIRE(styled.fontItalic());
    REQUIRE(styled.fontUnderline());
    REQUIRE(styled.fontLetterSpacing() == 3);
    REQUIRE(styled.foreground().color() == QColor(Qt::red));

    // Changing one property across mixed runs must keep the others mixed.
    richSelect(&item, 0, 19);
    size.setProperty(QTextFormat::FontPixelSize, 36);
    TitlerRichText::apply(&item, size);
    REQUIRE(richFormat(&item, 0).fontFamilies() == outside.fontFamilies());
    REQUIRE(richFormat(&item, 8).fontFamilies() == styled.fontFamilies());
    REQUIRE(richFormat(&item, 8).foreground().color() == QColor(Qt::red));
    REQUIRE(richFormat(&item, 8).fontWeight() == QFont::Bold);

    richSelect(&item, 13, 6);
    item.setAlignment(Qt::AlignHCenter);
    REQUIRE(item.textCursor().anchor() == 13);
    REQUIRE(item.textCursor().position() == 6);
    QTextCharFormat blue;
    blue.setForeground(QBrush(Qt::blue));
    TitlerRichText::apply(&item, blue);
    item.document()->undo();
    REQUIRE(richFormat(&item, 8).foreground().color() == QColor(Qt::red));
    item.document()->redo();
    REQUIRE(richFormat(&item, 8).foreground().color() == QColor(Qt::blue));

    // A caret-only change styles new typing, not the whole object.
    richSelect(&item, 9, 9);
    TitlerRichText::apply(&item, red);
    auto cursor = item.textCursor();
    cursor.insertText(QStringLiteral("X"));
    item.setTextCursor(cursor);
    REQUIRE(item.toPlainText() == QStringLiteral("Hello amaXzing world"));
    REQUIRE(richFormat(&item, 9).foreground().color() == QColor(Qt::red));
    REQUIRE(richFormat(&item, 0).foreground().color() == QColor(Qt::white));
}

TEST_CASE("Rich text paints mixed colors outside edit mode", "[Titler][RichText]")
{
    QGraphicsScene scene;
    auto *item = new MyTextItem(QStringLiteral("Hello world"), nullptr);
    scene.addItem(item);
    QTextCharFormat base, red;
    base.setProperty(QTextFormat::FontPixelSize, 48);
    base.setForeground(QBrush(Qt::white));
    richSelect(item, 0, 11);
    TitlerRichText::apply(item, base);
    richSelect(item, 6, 11);
    red.setForeground(QBrush(Qt::red));
    TitlerRichText::apply(item, red);
    richSelect(item, 0, 0);
    item->setTextInteractionFlags(Qt::NoTextInteraction);
    item->setSelected(false);
    QImage image(800, 200, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    scene.render(&painter, QRectF(image.rect()), scene.itemsBoundingRect().adjusted(-4, -4, 4, 4));
    painter.end();
    int reds = 0, whites = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor c = image.pixelColor(x, y);
            if (c.alpha() > 100) {
                reds += c.red() > 180 && c.green() < 80 && c.blue() < 80;
                whites += c.red() > 180 && c.green() > 180 && c.blue() > 180;
            }
        }
    }
    REQUIRE(reds > 10);
    REQUIRE(whites > 10);
}


TEST_CASE("Rich text survives title XML round trip", "[Titler][RichText]")
{
    MyTextItem source(QStringLiteral("Hello amazing world"), nullptr);
    source.setTextInteractionFlags(Qt::TextEditorInteraction);

    QTextCharFormat base;
    base.setFontFamilies(QStringList{QStringLiteral("serif")});
    base.setProperty(QTextFormat::FontPixelSize, 28);
    base.setFontWeight(QFont::Normal);
    base.setForeground(QBrush(Qt::white));

    richSelect(&source, 0, 19);
    TitlerRichText::apply(&source, base);

    QTextCharFormat styled;
    styled.setFontFamilies(QStringList{QStringLiteral("monospace")});
    styled.setProperty(QTextFormat::FontPixelSize, 48);
    styled.setFontWeight(QFont::Bold);
    styled.setFontItalic(true);
    styled.setFontUnderline(true);
    styled.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    styled.setFontLetterSpacing(3);
    styled.setForeground(QBrush(Qt::red));

    richSelect(&source, 6, 13);
    TitlerRichText::apply(&source, styled);

    QDomDocument stored = TitleDocument::xmlItem(&source, 1920, 1080);
    // RichText: serialize and parse XML, not just an in-memory DOM.
    const QString wire = stored.toString();
    REQUIRE(static_cast<bool>(stored.setContent(wire)));
    QDomElement itemElement = stored.documentElement();
    REQUIRE(itemElement.tagName() == QStringLiteral("item"));

    QDomElement content = itemElement.firstChildElement(QStringLiteral("content"));
    REQUIRE(!content.isNull());

    // Legacy fallback stays intact.
    REQUIRE(content.firstChild().nodeValue() == QStringLiteral("Hello amazing world"));

    QDomElement richText = content.firstChildElement(QStringLiteral("richtext"));
    REQUIRE(!richText.isNull());
    REQUIRE(richText.attribute(QStringLiteral("format")) == QStringLiteral("qt-html-v1"));
    REQUIRE(!richText.text().isEmpty());

    int missing = 0;
    int maxZ = 0;

    QScopedPointer<QGraphicsItem> loaded(
        TitleDocument::loadItemFromXml(
            itemElement,
            QString(),
            1920,
            1080,
            missing,
            maxZ));

    REQUIRE(!loaded.isNull());
    REQUIRE(loaded->type() == QGraphicsTextItem::Type);

    auto *text = static_cast<MyTextItem *>(loaded.data());
    REQUIRE(text->toPlainText() == QStringLiteral("Hello amazing world"));

    const QTextCharFormat outside = richFormat(text, 0);
    const QTextCharFormat inside = richFormat(text, 8);

    REQUIRE(outside.foreground().color() == QColor(Qt::white));
    REQUIRE(inside.foreground().color() == QColor(Qt::red));
    REQUIRE(inside.fontWeight() == QFont::Bold);
    REQUIRE(inside.fontItalic());
    REQUIRE(inside.fontUnderline());
    REQUIRE(inside.font().pixelSize() == 48);
    REQUIRE(inside.fontLetterSpacing() == Approx(3.0));
    REQUIRE(inside.fontLetterSpacingType() == QFont::AbsoluteSpacing);
    REQUIRE(content.attribute(QStringLiteral("font-pixel-size")).toInt() == 28);

    const QStringList insideFamilies = inside.fontFamilies().toStringList();
    REQUIRE(insideFamilies.contains(QStringLiteral("monospace")));
}


TEST_CASE("Rich text spacing handles Unicode and invalid ranges", "[Titler][RichText]")
{
    const QString sample = QStringLiteral("A\U0001F642\nB\te\u0301");
    MyTextItem source(sample, nullptr);
    source.setTextInteractionFlags(Qt::TextEditorInteraction);
    QTextCharFormat base;
    base.setProperty(QTextFormat::FontPixelSize, 28);
    base.setFontLetterSpacingType(QFont::PercentageSpacing);
    base.setFontLetterSpacing(110);
    richSelect(&source, 0, sample.size());
    TitlerRichText::apply(&source, base);
    QTextCharFormat fraction;
    fraction.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    fraction.setFontLetterSpacing(2.25);
    richSelect(&source, 1, 3); // One emoji, two UTF-16 units.
    TitlerRichText::apply(&source, fraction);
    fraction.setFontLetterSpacing(-0.5);
    const int pos = sample.indexOf(QLatin1Char('B'));
    richSelect(&source, pos, pos + 1);
    TitlerRichText::apply(&source, fraction);

    QDomDocument xml = TitleDocument::xmlItem(&source, 1920, 1080);
    const QString wire = xml.toString();
    REQUIRE(static_cast<bool>(xml.setContent(wire)));
    int missing = 0, maxZ = 0;
    QScopedPointer<QGraphicsItem> loaded(TitleDocument::loadItemFromXml(
        xml.documentElement(), QString(), 1920, 1080, missing, maxZ));
    REQUIRE(!loaded.isNull());
    auto *text = dynamic_cast<MyTextItem *>(loaded.data());
    REQUIRE(text != nullptr);
    REQUIRE(text->toPlainText() == sample);
    REQUIRE(richFormat(text, 0).fontLetterSpacingType() == QFont::PercentageSpacing);
    REQUIRE(richFormat(text, 0).fontLetterSpacing() == Approx(110));
    REQUIRE(richFormat(text, 1).fontLetterSpacing() == Approx(2.25));
    REQUIRE(richFormat(text, pos).fontLetterSpacing() == Approx(-0.5));

    QDomElement content = xml.documentElement().firstChildElement(QStringLiteral("content"));
    QDomElement data = content.firstChildElement(QStringLiteral("richtext-spacing"));
    REQUIRE(!data.isNull());
    data.firstChildElement(QStringLiteral("run")).setAttribute(QStringLiteral("start"), -1);
    const auto before = richFormat(text, pos);
    REQUIRE_FALSE(TitlerSpacingV1::restore(content, text->document()));
    REQUIRE(richFormat(text, pos) == before); // Reject without partial changes.
}


TEST_CASE("Rich text active inspector format follows caret and selection", "[Titler][RichText]")
{
    MyTextItem item(QStringLiteral("Hello amazing world"), nullptr);

    QTextCharFormat base;
    base.setFontFamilies(QStringList{QStringLiteral("serif")});
    base.setProperty(QTextFormat::FontPixelSize, 28);
    base.setFontWeight(QFont::Normal);
    base.setForeground(QBrush(Qt::white));

    richSelect(&item, 0, 19);
    TitlerRichText::apply(&item, base);

    QTextCharFormat styled;
    styled.setFontFamilies(QStringList{QStringLiteral("monospace")});
    styled.setProperty(QTextFormat::FontPixelSize, 48);
    styled.setFontWeight(QFont::Bold);
    styled.setFontItalic(true);
    styled.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    styled.setFontLetterSpacing(3);
    styled.setForeground(QBrush(Qt::red));

    richSelect(&item, 6, 13);
    TitlerRichText::apply(&item, styled);

    richSelect(&item, 8, 8);

    auto format =
        TitlerRichText::activeFormat(&item);

    REQUIRE(format.fontWeight() == QFont::Bold);
    REQUIRE(format.fontItalic());
    REQUIRE(format.foreground().color() == QColor(Qt::red));
    REQUIRE(format.fontLetterSpacing() == Approx(3.0));

    richSelect(&item, 6, 13);

    format =
        TitlerRichText::activeFormat(&item);

    REQUIRE(format.fontWeight() == QFont::Bold);
    REQUIRE(format.foreground().color() == QColor(Qt::red));

    REQUIRE_FALSE(
        TitlerRichText::selectionHasMixedCharacterFormat(&item));

    richSelect(&item, 0, 13);

    REQUIRE(
        TitlerRichText::selectionHasMixedCharacterFormat(&item));
}

// RichText: regression coverage for the actual rendering services.
#include <mlt++/MltFilter.h>
#include <mlt++/MltFrame.h>
#include <QCryptographicHash>
#include <QDir>
#include <QTextBoundaryFinder>

namespace {
QDomDocument makeRichTextTitle(const QString &text, int mode, int sigma = 0, bool shadow = false)
{
    MyTextItem source(text, nullptr);
    QFont defaultFont(QStringLiteral("serif"));
    defaultFont.setPixelSize(28);
    source.setFont(defaultFont);
    source.setPos(40, 40);
    source.setAlignment(Qt::AlignLeft);
    source.setData(TitleDocument::OutlineWidth, 0);
    source.updateShadow(shadow, 2, 8, 8, QColor(30, 210, 130, 128));
    source.updateTW(mode != 0, 2, mode, sigma, 37);
    QTextCharFormat base;
    base.setFont(defaultFont);
    base.setForeground(QBrush(Qt::white));
    richSelect(&source, 0, text.size());
    TitlerRichText::apply(&source, base);
    const int word = text.indexOf(QStringLiteral("amazing"));
    if (word >= 0) {
        QTextCharFormat style;
        style.setFontFamilies(QStringList{QStringLiteral("monospace")});
        style.setProperty(QTextFormat::FontPixelSize, 48);
        style.setFontWeight(QFont::Bold);
        style.setFontItalic(true);
        style.setFontUnderline(true);
        style.setForeground(QBrush(Qt::red));
        style.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        style.setFontLetterSpacing(3);
        richSelect(&source, word, word + 7);
        TitlerRichText::apply(&source, style);
    }
    richSelect(&source, 0, 0);
    auto item = TitleDocument::xmlItem(&source, 1280, 720);
    QDomDocument result;
    auto root = result.createElement(QStringLiteral("kdenlivetitle"));
    root.setAttribute(QStringLiteral("width"), 1280);
    root.setAttribute(QStringLiteral("height"), 720);
    root.setAttribute(QStringLiteral("out"), 299);
    result.appendChild(root);
    root.appendChild(result.importNode(item.documentElement(), true));
    return result;
}
void setProducerTitle(Mlt::Producer &producer, const QDomDocument &title)
{
    REQUIRE(producer.is_valid());
    producer.set("xmldata", title.toByteArray().constData());
    producer.set("length", 300);
    producer.set_in_and_out(0, 299);
    producer.set("force_reload", 1);
}
QImage renderProducerFrame(Mlt::Producer &producer, int position)
{
    producer.seek(position);
    std::unique_ptr<Mlt::Frame> frame(producer.get_frame());
    REQUIRE(frame.get() != nullptr);
    REQUIRE(frame->is_valid());
    mlt_image_format format = mlt_image_rgba;
    int width = 1280, height = 720;
    auto *data = frame->get_image(format, width, height);
    REQUIRE(data != nullptr);
    REQUIRE(format == mlt_image_rgba);
    REQUIRE(width == 1280);
    REQUIRE(height == 720);
    return QImage(data, width, height, width * 4, QImage::Format_RGBA8888).copy();
}
QByteArray frameHash(const QImage &image)
{
    return QCryptographicHash::hash(QByteArray(reinterpret_cast<const char *>(image.constBits()),
                                               image.sizeInBytes()), QCryptographicHash::Sha256);
}
int countRedPixels(const QImage &image)
{
    int count = 0;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            auto c = image.pixelColor(x, y);
            if (c.alpha() > 100 && c.red() > 180 && c.green() < 60 && c.blue() < 60) ++count;
        }
    }
    return count;
}
}

TEST_CASE("Rich text selection scan preserves Unicode and cursor", "[Titler][RichText]")
{
    const QString text = QString::fromUtf8("A\xCC\x88 \xF0\x9F\x8C\x88 amazing");
    MyTextItem item(text, nullptr);
    QTextCharFormat base;
    base.setForeground(QBrush(Qt::white));
    richSelect(&item, 0, text.size());
    TitlerRichText::apply(&item, base);
    REQUIRE_FALSE(TitlerRichText::selectionHasMixedCharacterFormat(&item));
    const int word = text.indexOf(QStringLiteral("amazing"));
    QTextCharFormat red;
    red.setForeground(QBrush(Qt::red));
    richSelect(&item, word, text.size());
    TitlerRichText::apply(&item, red);
    richSelect(&item, text.size(), 0);
    const auto html = item.toHtml();
    REQUIRE(TitlerRichText::selectionHasMixedCharacterFormat(&item));
    REQUIRE(item.textCursor().anchor() == text.size());
    REQUIRE(item.textCursor().position() == 0);
    REQUIRE(item.toHtml() == html);
}

TEST_CASE("Rich text colored shadow keeps premultiplied alpha", "[Titler][RichText]")
{
    QGraphicsScene scene;
    auto *item = new MyTextItem(QStringLiteral("Test"), nullptr);
    scene.addItem(item);
    QFont font(QStringLiteral("sans-serif"));
    font.setPixelSize(32);
    item->setFont(font);
    QTextCharFormat white;
    white.setForeground(QBrush(Qt::white));
    richSelect(item, 0, 4);
    TitlerRichText::apply(item, white);
    richSelect(item, 0, 0);
    item->setPos(20, 20);
    const QString html = item->toHtml();
    item->updateShadow(true, 2, 8, 50, QColor(0, 255, 0, 96));
    QImage image(320, 200, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    scene.render(&painter, QRectF(0, 0, 320, 200), QRectF(0, 0, 320, 200));
    painter.end();
    int green = 0;
    int invalid = 0;
    for (int y = 0; y < image.height(); ++y) {
        const QRgb *row = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            invalid += qRed(row[x]) > qAlpha(row[x]) || qGreen(row[x]) > qAlpha(row[x])
                || qBlue(row[x]) > qAlpha(row[x]);
            if (qAlpha(row[x]) > 10 && qAlpha(row[x]) <= 96 && qGreen(row[x]) > qRed(row[x])) ++green;
        }
    }
    REQUIRE(green > 10);
    REQUIRE(invalid == 0);
    REQUIRE(item->toHtml() == html);
}

TEST_CASE("Rich text native typewriter preserves render and seeking", "[Titler][RichTextRender]")
{
    Mlt::Profile profile("atsc_720p_25");
    const QString text = QString::fromUtf8("A\xCC\x88 Hello amazing } \\ world\nNext line");
    const auto plain = makeRichTextTitle(text, 0);
    Mlt::Producer reference(profile, "kdenlivetitle", "");
    setProducerTitle(reference, plain);
    const auto finalImage = renderProducerFrame(reference, 240);
    REQUIRE(countRedPixels(finalImage) > 10);
    const auto finalHash = frameHash(finalImage);
    for (int mode = 1; mode <= 3; ++mode) {
        INFO("native mode " << mode);
        Mlt::Producer animated(profile, "kdenlivetitle", "");
        const auto title = makeRichTextTitle(text, mode);
        setProducerTitle(animated, title);
        const QByteArray xmlBefore(animated.get("xmldata"));
        const auto first = renderProducerFrame(animated, 0);
        REQUIRE(frameHash(first) != finalHash);
        if (mode == 1) REQUIRE(countRedPixels(first) == 0);
        REQUIRE(frameHash(renderProducerFrame(animated, 240)) == finalHash);
        REQUIRE(frameHash(renderProducerFrame(animated, 0)) == frameHash(first));
        for (int pos : {20, 2, 240, 8, 0, 120, 4, 20}) {
            const auto a = frameHash(renderProducerFrame(animated, pos));
            renderProducerFrame(animated, 240);
            REQUIRE(frameHash(renderProducerFrame(animated, pos)) == a);
        }
        REQUIRE(QByteArray(animated.get("xmldata")) == xmlBefore);
    }
    // Reloading a title after disabling animation must clear the old state.
    Mlt::Producer changed(profile, "kdenlivetitle", "");
    setProducerTitle(changed, makeRichTextTitle(text, 1));
    renderProducerFrame(changed, 0);
    setProducerTitle(changed, plain);
    REQUIRE(frameHash(renderProducerFrame(changed, 0)) == finalHash);
    REQUIRE(changed.get_int("_animated") == 0);
}

TEST_CASE("Rich text effect stack typewriter preserves rich XML", "[Titler][RichTextRender]")
{
    Mlt::Profile profile("atsc_720p_25");
    const QString text = QStringLiteral("Hello amazing } \\ world\nSecond line");
    const auto title = makeRichTextTitle(text, 0);
    Mlt::Producer reference(profile, "kdenlivetitle", "");
    setProducerTitle(reference, title);
    const auto finalHash = frameHash(renderProducerFrame(reference, 240));
    for (int mode = 1; mode <= 3; ++mode) {
        INFO("effect stack mode " << mode);
        Mlt::Producer animated(profile, "kdenlivetitle", "");
        setProducerTitle(animated, title);
        Mlt::Filter effect(profile, "typewriter");
        REQUIRE(effect.is_valid());
        effect.set("macro_type", mode);
        effect.set("step_length", 2);
        effect.set("step_sigma", 0);
        effect.set("random_seed", 37);
        REQUIRE(animated.attach(effect) == 0);
        const QByteArray original(animated.get("xmldata"));
        const auto first = frameHash(renderProducerFrame(animated, 0));
        REQUIRE(first != finalHash);
        REQUIRE(QByteArray(animated.get("xmldata")) == original);
        REQUIRE(frameHash(renderProducerFrame(animated, 240)) == finalHash);
        REQUIRE(frameHash(renderProducerFrame(animated, 0)) == first);
        REQUIRE(QByteArray(animated.get("xmldata")) == original);
        // Timing changes must reparse from the ORIGINAL, not the last prefix.
        effect.set("step_length", 3);
        REQUIRE(frameHash(renderProducerFrame(animated, 240)) == finalHash);
        REQUIRE(QByteArray(animated.get("xmldata")) == original);
        renderProducerFrame(animated, 0);
        REQUIRE(animated.detach(effect) == 0);
        REQUIRE(frameHash(renderProducerFrame(animated, 0)) == finalHash);
    }
    // An unsupported producer must not leave the filter lock held.
    Mlt::Producer color(profile, "color", "white");
    REQUIRE(color.is_valid());
    Mlt::Filter unsupported(profile, "typewriter");
    REQUIRE(unsupported.is_valid());
    REQUIRE(color.attach(unsupported) == 0);
    REQUIRE(!renderProducerFrame(color, 0).isNull());
    REQUIRE(!renderProducerFrame(color, 1).isNull());
}

TEST_CASE("Rich text seeded timing and shadow are seek repeatable", "[Titler][RichTextRender]")
{
    Mlt::Profile profile("atsc_720p_25");
    const auto title = makeRichTextTitle(QStringLiteral("Hello amazing world"), 1, 3, true);
    Mlt::Producer first(profile, "kdenlivetitle", ""), second(profile, "kdenlivetitle", "");
    setProducerTitle(first, title);
    setProducerTitle(second, title);
    const auto staticTitle = makeRichTextTitle(QStringLiteral("Hello amazing world"), 0, 0, true);
    Mlt::Producer reference(profile, "kdenlivetitle", "");
    setProducerTitle(reference, staticTitle);
    REQUIRE(frameHash(renderProducerFrame(first, 240)) == frameHash(renderProducerFrame(reference, 240)));
    for (int repeat = 0; repeat < 3; ++repeat) {
        for (int pos : {0, 24, 2, 240, 6, 18, 0, 9}) {
            INFO("seeded seek " << pos << " repeat " << repeat);
            REQUIRE(frameHash(renderProducerFrame(first, pos)) == frameHash(renderProducerFrame(second, pos)));
        }
    }
}

#include <QFile>
#include <QTemporaryDir>
TEST_CASE("Rich text file backed typewriter restores the source", "[Titler][RichTextRender]")
{
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    const auto title = makeRichTextTitle(QStringLiteral("Hello amazing world"), 0);
    const auto bytes = title.toByteArray();
    const auto filename = directory.filePath(QStringLiteral("title.kdenlivetitle"));
    QFile file(filename);
    REQUIRE(file.open(QIODevice::WriteOnly));
    REQUIRE(file.write(bytes) == bytes.size());
    file.close();
    Mlt::Profile profile("atsc_720p_25");
    Mlt::Producer reference(profile, "kdenlivetitle", "");
    setProducerTitle(reference, title);
    const auto finalHash = frameHash(renderProducerFrame(reference, 240));
    Mlt::Producer animated(profile, "kdenlivetitle", filename.toUtf8().constData());
    REQUIRE(animated.is_valid());
    animated.set("length", 300);
    animated.set_in_and_out(0, 299);
    Mlt::Filter effect(profile, "typewriter");
    REQUIRE(effect.is_valid());
    effect.set("macro_type", 1);
    effect.set("step_length", 2);
    effect.set("step_sigma", 0);
    effect.set("random_seed", 37);
    REQUIRE(animated.attach(effect) == 0);
    REQUIRE(frameHash(renderProducerFrame(animated, 0)) != finalHash);
    REQUIRE(QByteArray(animated.get("_xmldata")) == bytes);
    REQUIRE(frameHash(renderProducerFrame(animated, 240)) == finalHash);
    REQUIRE(QByteArray(animated.get("_xmldata")) == bytes);
}

TEST_CASE("Rich text emits reference frames for lossless export", "[Titler][RichTextRender]")
{
    const auto title = makeRichTextTitle(QStringLiteral("Hello amazing world"), 1);
    Mlt::Profile profile("atsc_720p_25");
    Mlt::Producer producer(profile, "kdenlivetitle", "");
    setProducerTitle(producer, title);
    const QString output = qEnvironmentVariable("RICHTEXT_TEST_ARTIFACTS");
    if (!output.isEmpty()) {
        QDir dir(output);
        REQUIRE(dir.exists());
        QFile file(dir.filePath(QStringLiteral("native.kdenlivetitle")));
        REQUIRE(file.open(QIODevice::WriteOnly));
        const auto bytes = title.toByteArray();
        REQUIRE(file.write(bytes) == bytes.size());
    }
    for (int frame : {0, 20, 39}) {
        const auto image = renderProducerFrame(producer, frame);
        REQUIRE(!image.isNull());
        if (!output.isEmpty()) {
            QFile file(QDir(output).filePath(QStringLiteral("reference-%1.rgba").arg(frame)));
            REQUIRE(file.open(QIODevice::WriteOnly));
            REQUIRE(file.write(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes()) == image.sizeInBytes());
            REQUIRE(image.save(QDir(output).filePath(QStringLiteral("reference-%1.png").arg(frame))));
        }
    }
}


TEST_CASE("Rich text selective gradient survives XML round trip", "[Titler][RichText]")
{
    MyTextItem source(QStringLiteral("Hello amazing world"), nullptr);
    QFont font(QStringLiteral("sans-serif"));
    font.setPixelSize(32);
    source.setFont(font);
    QTextCharFormat base;
    base.setForeground(QBrush(Qt::white));
    richSelect(&source, 0, source.toPlainText().size());
    TitlerRichText::apply(&source, base);
    const QString data = QStringLiteral("#ffff0000;#ff0000ff;0;100;0");
    const auto rect = source.boundingRect();
    QTextCharFormat gradient;
    gradient.setProperty(TitlerGradientV1::Property, data);
    gradient.setForeground(QBrush(TitlerGradientV1::gradientFromString(data, int(rect.width()), int(rect.height()))));
    richSelect(&source, 6, 13);
    TitlerRichText::apply(&source, gradient);
    richSelect(&source, 0, 0);

    const QDomDocument saved = TitleDocument::xmlItem(&source, 1280, 720);
    const QDomElement content = saved.documentElement().firstChildElement(QStringLiteral("content"));
    REQUIRE(content.attribute(QStringLiteral("gradient")).isEmpty());
    const QDomElement supplement = content.firstChildElement(QStringLiteral("richtext-gradients"));
    REQUIRE_FALSE(supplement.isNull());
    const QDomElement run = supplement.firstChildElement(QStringLiteral("run"));
    REQUIRE(run.attribute(QStringLiteral("start")).toInt() == 6);
    REQUIRE(run.attribute(QStringLiteral("length")).toInt() == 7);

    int missing = 0;
    int maxZ = 0;
    QScopedPointer<QGraphicsItem> loaded(TitleDocument::loadItemFromXml(
        saved.documentElement(), QString(), 1280, 720, missing, maxZ));
    REQUIRE(loaded);
    auto *text = static_cast<MyTextItem *>(loaded.data());
    REQUIRE(richFormat(text, 0).property(TitlerGradientV1::Property).toString().isEmpty());
    REQUIRE(richFormat(text, 7).property(TitlerGradientV1::Property).toString() == data);
    REQUIRE(richFormat(text, 14).property(TitlerGradientV1::Property).toString().isEmpty());
    REQUIRE(richFormat(text, 0).foreground().style() == Qt::SolidPattern);
    REQUIRE(richFormat(text, 7).foreground().style() == Qt::LinearGradientPattern);
}

TEST_CASE("Rich text selective gradient renders in MLT and typewriter", "[Titler][RichText][RichTextRender]")
{
    const auto makeTitle = [](int mode) {
        MyTextItem source(QStringLiteral("Hello amazing world"), nullptr);
        QFont font(QStringLiteral("sans-serif"));
        font.setPixelSize(40);
        source.setFont(font);
        source.setPos(40, 40);
        source.setAlignment(Qt::AlignLeft);
        source.setData(TitleDocument::OutlineWidth, 0);
        source.updateTW(mode != 0, 2, mode, 0, 37);
        QTextCharFormat base;
        base.setForeground(QBrush(Qt::white));
        richSelect(&source, 0, source.toPlainText().size());
        TitlerRichText::apply(&source, base);
        const QString data = QStringLiteral("#ffff0000;#ff0000ff;0;100;0");
        const auto rect = source.boundingRect();
        QTextCharFormat gradient;
        gradient.setProperty(TitlerGradientV1::Property, data);
        gradient.setForeground(QBrush(TitlerGradientV1::gradientFromString(data, int(rect.width()), int(rect.height()))));
        richSelect(&source, 6, 13);
        TitlerRichText::apply(&source, gradient);
        richSelect(&source, 0, 0);
        auto item = TitleDocument::xmlItem(&source, 1280, 720);
        QDomDocument result;
        auto root = result.createElement(QStringLiteral("kdenlivetitle"));
        root.setAttribute(QStringLiteral("width"), 1280);
        root.setAttribute(QStringLiteral("height"), 720);
        root.setAttribute(QStringLiteral("out"), 299);
        result.appendChild(root);
        root.appendChild(result.importNode(item.documentElement(), true));
        return result;
    };

    Mlt::Profile profile("atsc_720p_25");
    Mlt::Producer reference(profile, "kdenlivetitle", "");
    setProducerTitle(reference, makeTitle(0));
    const QImage finalImage = renderProducerFrame(reference, 240);
    int white = 0;
    int colored = 0;
    for (int y = 0; y < finalImage.height(); ++y) {
        for (int x = 0; x < finalImage.width(); ++x) {
            const QColor c = finalImage.pixelColor(x, y);
            if (c.alpha() < 80) continue;
            if (c.red() > 180 && c.green() > 180 && c.blue() > 180) ++white;
            if ((c.red() > 120 || c.blue() > 120) && c.green() < 140
                && qAbs(c.red() - c.blue()) > 20) ++colored;
        }
    }
    REQUIRE(white > 20);
    REQUIRE(colored > 20);

    Mlt::Producer animated(profile, "kdenlivetitle", "");
    setProducerTitle(animated, makeTitle(1));
    REQUIRE(frameHash(renderProducerFrame(animated, 0)) != frameHash(finalImage));
    REQUIRE(frameHash(renderProducerFrame(animated, 240)) == frameHash(finalImage));
    REQUIRE(frameHash(renderProducerFrame(animated, 0)) != frameHash(finalImage));
}
