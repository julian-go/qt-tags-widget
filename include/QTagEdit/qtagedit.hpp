#ifndef QTAGEDIT_Q_TAG_EDIT_H_
#define QTAGEDIT_Q_TAG_EDIT_H_

#include <QLineEdit>
#include <functional>
#include <memory>

class QKeyEvent;
class QPen;
class QColor;
class QStylePainter;

class QTagEdit : public QLineEdit {
  Q_OBJECT

 public:
  /// @brief A property is a tag with a list of values
  struct Property {
    QString name;
    QStringList values;
  };

  using PropertyList = QList<Property>;

  struct Style {
    QColor line_color;
    QColor shade_color;
    QColor property_color;
  };

  QTagEdit(QWidget *parent = nullptr);
  ~QTagEdit();

  /// @brief Sets the tags
  void setTags(const QStringList &tags);

  /// @brief Sets the tags for completion
  void setTagsForCompletion(const QStringList &tags);

  /// @brief Returns the tags
  /// @returns The tags as a list of strings
  QStringList getTags() const;

  /// @brief Appends a single tag
  void addTag(const QString &tag);

  /// @brief Removes the last tag
  void removeLastTag();

  /// @brief Sets the tags as a list of properties
  ///
  /// It only makes sense to use this function if the property separator has
  /// been set.
  void setProperties(const PropertyList &properties);

  /// @brief Appends a single property
  ///
  /// It only makes sense to use this function if the property separator has
  /// been set.
  void addProperty(const Property &property);

  /// @brief Returns the tags as a list of properties.
  ///
  /// It only makes sense to use this function if the property separator has
  /// been set.
  /// @return The tags as a list of properties with their associated values
  PropertyList getProperties() const;

  /// @brief Sets the primary colors
  /// @param line_color The color to be used to render the underline
  /// @param shade_color The color to be used to render the tag background
  /// @param property_color The color to be used to render the tag property
  void setColors(const QColor &line_color, const QColor &shade_color,
                 const QColor &property_color);

  /// @brief Sets the secondary colors
  /// @param line_color The color to be used to render the underline
  /// @param shade_color The color to be used to render the tag background
  /// @param property_color The color to be used to render the tag property
  void setSecondaryColors(const QColor &line_color, const QColor &shade_color,
                          const QColor &property_color);

  /// @brief Sets the tag filter
  ///
  /// If a tag matches the filter it is rendered with the default color,
  /// otherwise it is rendered with the secondary color.
  /// @param filter The filter function
  void setTagFilter(std::function<bool(const QString &)> filter);

  /// @brief Sets the property separator
  ///
  /// When set tags are rendered as properties with a name and a list of
  /// values seperated by the given character.
  ///
  /// @param separator The character to be used as a separator
  void setPropertySeparator(QChar separator);

  /// @brief Sets tags to be unique
  ///
  /// If unique is set to true, tags will be collapsed to be unique
  void setUniqueTags(bool unique);

  /// @brief overriden sizeHint
  QSize sizeHint() const override;

  /// @brief overriden minimumSizeHint
  QSize minimumSizeHint() const override;

 signals:
  /// @brief This signal is emitted whenever the tags change, even when done so
  /// programmatically
  void tagsChanged();

  /// @brief This signal is emitted whenever the tags are edited by the user
  void tagsEdited();

 protected:
  void paintEvent(QPaintEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

 private slots:
  void insertCompletion(const QString &completion);

 private:
  void renderTags(QStylePainter &painter, QRect rect);
  void renderTagBackgrounds(QStylePainter &painter, QRect rect, bool line_only);
  QPen getPenForColor(const QColor &color);
  bool Filter(const QString &tag);
  void makeTagsUnique();

  struct Impl;
  std::unique_ptr<Impl> impl;
};

#endif  // QTAGEDIT_Q_TAG_EDIT_H_