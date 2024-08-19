// QTagEdit
// Copyright (C) 2024  Julian Gottwald
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 3 as
// published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this library.  If not, see <https://www.gnu.org/licenses/>.
#include "qtagedit.hpp"

#include <QBrush>
#include <QColor>
#include <QCompleter>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpressionValidator>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <algorithm>
#include <optional>

struct QTagEdit::Impl {
  static constexpr int kLineEditLeftMargin{3};

  // Colors for tag brackgrounds
  // https://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale
  static constexpr double kRgbBrightnessWeights[] = {0.299, 0.587, 0.114};
  static constexpr double kDarkColorTreshold = 150;
  static constexpr QColor kDarkColor{0, 0, 0};
  static constexpr QColor kBrightColor{245, 245, 245};

  static constexpr int kLeftMargin = 0;
  static constexpr int kTopMargin = 0;
  static constexpr int kRightMargin = 0;
  static constexpr int kBottomMargin = 2;
  static constexpr QMargins kTagMargins{kLeftMargin, kTopMargin, kRightMargin,
                                        kBottomMargin};
  static constexpr QMargins kTagMarginsWithProperty{kLeftMargin, kTopMargin, 0,
                                                    kBottomMargin};
  static constexpr QMargins kPropertyMargins{0, kTopMargin, kRightMargin,
                                             kBottomMargin};
  static constexpr int kLineWidth = 2;
  static constexpr int kAdditionalBottomMargin = 2;

  static constexpr QColor kLineColor{37, 150, 190, 255};
  static constexpr QColor kShadeColor{37, 150, 190, 127};
  static constexpr QColor kPropertyColor{37, 150, 190, 90};

  static constexpr QColor kSecondaryLineColor{190, 155, 37, 255};
  static constexpr QColor kSecondaryShadeColor{190, 155, 37, 127};
  static constexpr QColor kSecondaryPropertyColor{190, 155, 37, 90};

  Style primary{.line_color = kLineColor,
                .shade_color = kShadeColor,
                .property_color = kPropertyColor};
  Style secondary{.line_color = kSecondaryLineColor,
                  .shade_color = kSecondaryShadeColor,
                  .property_color = kSecondaryPropertyColor};

  std::optional<QChar> separator{};

  std::function<bool(const QString &)> tag_filter{};

  bool unique_tags{true};

  std::unique_ptr<QCompleter> completer{nullptr};
};

QTagEdit::QTagEdit(QWidget *parent)
    : QLineEdit(parent), impl{std::make_unique<Impl>()}
{
  connect(this, &QLineEdit::textChanged, this, &QTagEdit::tagsChanged);
  connect(this, &QLineEdit::textEdited, this, &QTagEdit::tagsEdited);
  connect(this, &QLineEdit::editingFinished, this, &QTagEdit::makeTagsUnique);

  // Only allow a single whitespace between tags
  this->setValidator(
      new QRegularExpressionValidator(QRegularExpression("\\S+(\\s\\S+)*")));
}

QTagEdit::~QTagEdit() {}

void QTagEdit::setTags(const QStringList &tags) { setText(tags.join(" ")); }

void QTagEdit::setTagsForCompletion(const QStringList &tags)
{
  impl->completer = std::make_unique<QCompleter>(tags);
  impl->completer->setCaseSensitivity(Qt::CaseInsensitive);
  impl->completer->setWidget(this);
  connect(impl->completer.get(),
          QOverload<const QString &>::of(&QCompleter::activated), this,
          [this](QString const &text) {
            removeLastTag();
            addTag(text);
          });
}

QStringList QTagEdit::getTags() const
{
  return text().split(" ", Qt::SkipEmptyParts);
}

void QTagEdit::addTag(const QString &tag)
{
  if (this->text().isEmpty()) {
    this->setText(tag);
  } else {
    this->setText(this->text() + " " + tag);
  }
}

void QTagEdit::removeLastTag()
{
  auto text = this->text();
  auto index = text.lastIndexOf(' ');
  if (index >= 0) {
    text.truncate(index);
    this->setText(text);
  } else {
    this->setText("");
  }
}

void QTagEdit::setProperties(const PropertyList &properties)
{
  auto tags = QStringList{};
  for (auto property : properties) {
    tags.append(property.name);
    if (auto sep = impl->separator) {
      for (auto value : property.values) {
        tags.last() += *sep + value;
      }
    }
  }
  setTags(tags);
}

void QTagEdit::addProperty(const Property &property)
{
  auto tag = property.name;
  if (auto sep = impl->separator) {
    for (auto value : property.values) {
      tag += *sep + value;
    }
  }
  this->setText(this->text() + " " + tag);
}

QTagEdit::PropertyList QTagEdit::getProperties() const
{
  auto list = PropertyList{};
  auto tags = getTags();
  for (auto tag : tags) {
    if (auto sep = impl->separator) {
      auto tokens = tag.split(*sep);
      if (tokens.size() > 1) {
        list.append({.name = tokens[0], .values = tokens.mid(1)});
      } else {
        list.append({.name = tokens[0], .values = {}});
      }
    }
  }
  return list;
}

void QTagEdit::setColors(const QColor &line_color, const QColor &shade_color,
                         const QColor &property_color)
{
  impl->primary.line_color = line_color;
  impl->primary.shade_color = shade_color;
  impl->primary.property_color = property_color;
}

void QTagEdit::setSecondaryColors(const QColor &line_color,
                                  const QColor &shade_color,
                                  const QColor &property_color)
{
  impl->secondary.line_color = line_color;
  impl->secondary.shade_color = shade_color;
  impl->secondary.property_color = property_color;
}

void QTagEdit::setTagFilter(std::function<bool(const QString &)> filter)
{
  impl->tag_filter = filter;
}

void QTagEdit::setPropertySeparator(QChar separator)
{
  impl->separator = separator;
}

void QTagEdit::setUniqueTags(bool unique) { impl->unique_tags = unique; }

QSize QTagEdit::sizeHint() const
{
  auto size = QLineEdit::sizeHint();
  if constexpr (Impl::kTagMargins.top() > Impl::kTagMargins.bottom()) {
    size.setHeight(size.height() + impl->kTagMargins.top() * 2 +
                   Impl::kAdditionalBottomMargin);
  } else {
    size.setHeight(size.height() + impl->kTagMargins.bottom() * 2 +
                   Impl::kAdditionalBottomMargin);
  }
  return size;
}

QSize QTagEdit::minimumSizeHint() const
{
  auto size = QLineEdit::minimumSizeHint();
  if constexpr (Impl::kTagMargins.top() > Impl::kTagMargins.bottom()) {
    size.setHeight(size.height() + impl->kTagMargins.top() * 2 +
                   Impl::kAdditionalBottomMargin);
  } else {
    size.setHeight(size.height() + impl->kTagMargins.bottom() * 2 +
                   Impl::kAdditionalBottomMargin);
  }
  return size;
}

void QTagEdit::paintEvent(QPaintEvent *event)
{
  QStyleOptionFrame text_frame;
  text_frame.initFrom(this);

  QStyleOptionFocusRect focus_rect;
  focus_rect.initFrom(this);

  auto content_rect =
      style()->subElementRect(QStyle::SE_LineEditContents, &text_frame, this);
  content_rect.translate(impl->kLineEditLeftMargin, 0);

  if (hasFocus()) {
    QLineEdit::paintEvent(event);

    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    renderTagBackgrounds(painter, content_rect, true);
    return;
  } else {
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPrimitive(QStyle::PE_PanelLineEdit, text_frame);
    painter.drawPrimitive(QStyle::PE_FrameLineEdit, focus_rect);
    renderTagBackgrounds(painter, content_rect, false);
    renderTags(painter, content_rect);
  }
}

void QTagEdit::keyPressEvent(QKeyEvent *event)
{
  QLineEdit::keyPressEvent(event);

  if (impl->completer != nullptr) {
    if (this->text().isEmpty() || this->text().back() == ' ') {
      impl->completer->setCompletionPrefix("");
      impl->completer->complete();
    } else {
      auto tags = getTags();
      if (!tags.isEmpty()) {
        auto last_tag = tags.back();
        impl->completer->setCompletionPrefix(last_tag);
        impl->completer->complete();
      }
    }
  }
}

void QTagEdit::insertCompletion(const QString &completion)
{
  if (impl->completer == nullptr) {
    return;
  }

  QCursor tc = cursor();
}

void QTagEdit::renderTags(QStylePainter &painter, QRect rect)
{
  for (auto tag : getTags()) {
    this->ensurePolished();

    auto pen = Filter(tag) ? getPenForColor(impl->primary.property_color)
                           : getPenForColor(impl->secondary.property_color);
    if (!this->isEnabled()) {
      pen.setColor(QColor("gray"));
    }
    painter.setPen(pen);
    painter.drawText(rect, Qt::AlignVCenter, tag);

    rect.moveLeft(rect.left() + fontMetrics().horizontalAdvance(tag + " "));
  }
}

void QTagEdit::renderTagBackgrounds(QStylePainter &painter, QRect rect,
                                    bool line_only)
{
  auto text_y =
      static_cast<int>(rect.height() / 2.0 + fontMetrics().height() / 2.0);
  auto text_rect = [&](const QString &str, int offset,
                       QMargins margin) -> QRect {
    auto rect = QRect{0, 0, fontMetrics().horizontalAdvance(str),
                      fontMetrics().height()};
    rect.moveBottom(text_y);
    rect.moveLeft(offset);
    rect += margin;
    return rect;
  };

  for (auto tag : getTags()) {
    this->ensurePolished();

    QString tag_only = tag;
    QString property_only = "";
    auto style = Filter(tag) ? impl->primary : impl->secondary;
    if (impl->separator) {
      auto first_sep = tag.indexOf(*impl->separator);
      if (first_sep >= 0) {
        tag_only.truncate(first_sep);
        style = Filter(tag_only) ? impl->primary : impl->secondary;
        property_only = tag.sliced(first_sep);
      }
    }
    if (!line_only && this->isEnabled()) {
      auto has_property = !property_only.isEmpty();
      auto margin =
          has_property ? Impl::kTagMarginsWithProperty : Impl::kTagMargins;
      QPainterPath path;
      path.addRect(text_rect(tag, rect.left(), margin));
      painter.fillPath(path, style.shade_color);

      if (has_property) {
        QPainterPath path;
        int offset = rect.left() + fontMetrics().horizontalAdvance(tag_only);
        path.addRect(text_rect(property_only, offset, Impl::kPropertyMargins));
        painter.fillPath(path, style.property_color);
      }
    }
    {
      auto line_rect = text_rect(tag, rect.left(), Impl::kTagMargins);
      if (this->isEnabled()) {
        painter.setPen(QPen(style.line_color, Impl::kLineWidth));
      } else {
        painter.setPen(QPen(QColor("lightgray"), Impl::kLineWidth));
      }
      painter.drawLine(line_rect.bottomLeft(), line_rect.bottomRight());
    }
    rect.moveLeft(rect.left() + fontMetrics().horizontalAdvance(tag + " "));
  }
}

QPen QTagEdit::getPenForColor(const QColor &color)
{
  auto scale_a = [&color](int value) {
    return 255 - color.alpha() / 255.0 * (255.0 - value);
  };
  const double weighted_color =
      scale_a(color.red()) * impl->kRgbBrightnessWeights[0] +
      scale_a(color.green()) * impl->kRgbBrightnessWeights[1] +
      scale_a(color.blue()) * impl->kRgbBrightnessWeights[2];
  if (weighted_color > impl->kDarkColorTreshold) {
    return QPen(impl->kDarkColor);
  } else {
    return QPen(impl->kBrightColor);
  }
}

bool QTagEdit::Filter(const QString &tag)
{
  if (impl->tag_filter) {
    return impl->tag_filter(tag);
  } else {
    return true;
  }
}

void QTagEdit::makeTagsUnique()
{
  if (!impl->unique_tags) {
    return;
  }
  if (impl->separator) {
    auto properties = getProperties();
    auto last = std::unique(
        properties.begin(), properties.end(),
        [](const Property &a, const Property &b) { return a.name == b.name; });
    properties.erase(last, properties.end());
    setProperties(properties);
  } else {
    auto tags = getTags();
    auto last =
        std::unique(tags.begin(), tags.end(),
                    [](const QString &a, const QString &b) { return a == b; });
    tags.erase(last, tags.end());
    setTags(tags);
  }
}
