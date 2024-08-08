#include <QDebug>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QMainWindow>
#include <memory>

#include "qtagedit.hpp"

std::unique_ptr<QMainWindow> setupUi()
{
  auto main_window = std::make_unique<QMainWindow>();
  main_window->setCentralWidget(new QWidget());
  main_window->resize(800, 200);
  auto layout = new QFormLayout(main_window->centralWidget());

  auto tag_edit = new QTagEdit();
  auto property_edit = new QTagEdit();
  auto line_edit = new QLineEdit();

  {
    QStringList valid_tags = {"wow", "such", "tags"};
    tag_edit->setTagsForCompletion(valid_tags);
    tag_edit->setTagFilter(
        [valid_tags](const QString& tag) { return valid_tags.contains(tag); });
    layout->addRow("Tags", tag_edit);
  }

  {
    QStringList valid_properties = {"width", "height", "box"};
    property_edit->setTagsForCompletion(valid_properties);
    property_edit->setTagFilter([valid_properties](const QString& tag) {
      return valid_properties.contains(tag);
    });
    property_edit->setPropertySeparator('=');
    layout->addRow("Properties", property_edit);
  }

  {
    layout->addRow("Line edit", line_edit);
    QObject::connect(property_edit, &QTagEdit::tagsChanged,
                     [property_edit, line_edit]() {
                       auto properties = property_edit->getProperties();
                       QString text{};
                       for (auto prop : properties) {
                         text.append(prop.name + ", ");
                       }
                       text.chop(2);
                       line_edit->setText(text);
                     });
  }

  return main_window;
}

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  auto main_window = setupUi();
  main_window->show();
  return a.exec();
}
