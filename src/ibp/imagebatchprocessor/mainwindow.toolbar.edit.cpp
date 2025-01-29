//
// MIT License
// 
// Copyright (c) Deif Lou
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMenu>
#include <QInputDialog>
#include <QDebug>
#include <algorithm>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../imgproc/freeimage.h"
#include "../misc/configurationmanager.h"
#include "../widgets/filedialog.h"

using namespace ibp::misc;
using namespace ibp::widgets;

void MainWindow::toolbarEditLoad()
{
    // Save Image Button
    ui->mToolbarEditButtonSaveImage->setEnabled(!mViewEditOutputImage.isNull());

    // Add Filter Button
    if (mMainImageFilterPluginLoader.count() > 0)
        toolbarEditPopulateAddFilterMenu();
    else
        ui->mToolbarEditButtonAddFilter->setEnabled(false);

    // Load Filters Button
    if (mMainImageFilterPluginLoader.count() == 0)
        ui->mToolbarEditButtonLoadFilters->setEnabled(false);
    toolbarEditReloadImageFilterListPresets();

    // Save Filters Button
    ui->mToolbarEditButtonSaveFilters->setEnabled(false);

}

void MainWindow::toolbarEditUnload()
{

}

bool toolbarEditAddFilterActionSorter(QAction * a1, QAction * a2)
{
    QString s1 = a1->text();
    QString s2 = a2->text();
    return s1.toLower() < s2.toLower();
}
void MainWindow::toolbarEditPopulateAddFilterMenu()
{
    int i, j, k;
    QString id;
    QStringList ids = mMainImageFilterPluginLoader.ids();
    QHash<QString, QString> info;
    QAction * action;
    QList<QAction *> actions;
    QStringList tags, filterTags;
    for (i = 0; i < ids.size(); i++)
    {
        id = ids.at(i);
        info = mMainImageFilterPluginLoader.info(id);

        action = new QAction(this);
        action->setText(info.value("name"));
        action->setProperty("id", id);
        action->setProperty("tags", info.value("tags"));
        action->setProperty("type", "action");
        action->setToolTip("<h4>" + info.value("name") + "</h4>" +
                           "<p>" + info.value("description") + "</p>");
        connect(action, SIGNAL(triggered()), this, SLOT(On_mToolbarEditButtonAddFilterAction_triggered()));
        actions.append(action);
        filterTags = info.value("tags").split(QRegExp("\\s*,\\s*"), Qt::SkipEmptyParts);
        for (j = 0; j < filterTags.size(); j++)
            if (!tags.contains(filterTags.at(j), Qt::CaseInsensitive))
                tags.append(filterTags.at(j));
    }
    if (actions.isEmpty())
        return;

    std::sort(actions.begin(), actions.end(), toolbarEditAddFilterActionSorter);
    tags.sort(Qt::CaseInsensitive);

    QMenu * mainMenu = new QMenu(this);
    ui->mToolbarEditButtonAddFilter->setMenu(mainMenu);
    for (i = 0; i < tags.size(); i++)
        mainMenu->addMenu(tags.at(i));
    if (!tags.isEmpty())
        mainMenu->addSeparator();
    QMenu * allFiltersMenu = mainMenu->addMenu(tr("All Filters"));

    for (i = 0; i < actions.size(); i++)
    {
        filterTags = actions.at(i)->property("tags").toString().split(QRegExp("\\s*,\\s*"), Qt::SkipEmptyParts);
        for (j = 0; j < filterTags.size(); j++)
        {
            for (k = 0; k < tags.size(); k++)
                if (mainMenu->actions().at(k)->text().compare(filterTags.at(j), Qt::CaseInsensitive) == 0)
                {
                    mainMenu->actions().at(k)->menu()->addAction(actions.at(i));
                    break;
                }
        }
        allFiltersMenu->addAction(actions.at(i));
    }

}

void MainWindow::toolbarEditReloadImageFilterListPresets()
{
    if (mMainImageFilterListPresets.size() == 0 || mMainImageFilterPluginLoader.count() == 0)
    {
        ui->mToolbarEditButtonLoadFilters->setPopupMode(QToolButton::DelayedPopup);
        return;
    }
    ui->mToolbarEditButtonLoadFilters->setPopupMode(QToolButton::MenuButtonPopup);

    QMenu * menu = new QMenu(this);
    QAction * action;
    for (int i = 0; i < mMainImageFilterListPresets.size(); i++)
    {
        action = new QAction(QString(mMainImageFilterListPresets.at(i).name).replace("&", "&&"), menu);
        action->setProperty("fileName", mMainImageFilterListPresets.at(i).fileName);
        connect(action, SIGNAL(triggered()), this, SLOT(On_mToolbarEditButtonLoadFiltersAction_triggered()));
        menu->addAction(action);
    }

    ui->mToolbarEditButtonLoadFilters->setMenu(menu);

    if (mToolbarEditButtonLoadFiltersMenu)
        delete mToolbarEditButtonLoadFiltersMenu;
    mToolbarEditButtonLoadFiltersMenu = menu;
}

void MainWindow::on_mToolbarEditButtonLoadImage_clicked()
{
    QString fileName;
    fileName = getOpenFileName(this, "images", freeimageGetOpenFilterString());

    if (fileName.isEmpty())
        return;

    if (!viewEditLoadInputImage(fileName))
        QMessageBox::information(this, QString(), tr("The selected file has an unsupported format."));
}

void MainWindow::on_mToolbarEditButtonSaveImage_clicked()
{
    QString fileName, filter;
    fileName = getSaveFileName(this, "images", freeimageGetSaveFilterString(), &filter);

    if (fileName.isEmpty())
        return;

    if (!viewEditSaveOutputImage(fileName, filter))
        QMessageBox::information(this, QString(), tr("An error has happen while saving the file to disk."));
}

void MainWindow::on_mToolbarEditButtonLoadFilters_clicked()
{
    if (mViewEditImageFilterListIsDirty)
        if (QMessageBox::question(this, QString(), tr("Do you want to save the current filter "
                                                      "list before loading the new one?")) == QMessageBox::Yes)
            if (!on_mToolbarEditButtonSaveFilters_clicked())
                return;

    QString fileName;
    fileName = getOpenFileName(this, "imagefilterlists", tr("IBP Image Filter List (*.ifl);;") +
                                                         tr("All Files (*)"));

    if (fileName.isEmpty())
        return;


    if (!viewEditLoadImageFilterList(fileName))
    {
        QMessageBox::information(this, QString(), tr("The selected file has an unsupported format."));
        return;
    }

    mViewEditImageFilterListIsDirty = false;
}

void MainWindow::On_mToolbarEditButtonLoadFiltersAction_triggered()
{
    QString fileName = sender()->property("fileName").toString();

    if (!QFile::exists(fileName))
    {
        QMessageBox::information(this, QString(), tr("The selected file doesn't exist."));
        return;
    }

    if (mViewEditImageFilterListIsDirty)
        if (QMessageBox::question(this, QString(), tr("Do you want to save the current filter "
                                                      "list before loading the new one?")) == QMessageBox::Yes)
            if (!on_mToolbarEditButtonSaveFilters_clicked())
                return;

    if (!viewEditLoadImageFilterList(fileName))
    {
        QMessageBox::information(this, QString(), tr("The selected file has an unsupported format."));
        return;
    }

    mViewEditImageFilterListIsDirty = false;
}

bool MainWindow::on_mToolbarEditButtonSaveFilters_clicked()
{
    QString name, description, fileName;
    bool ok;

    name = QInputDialog::getText(this, QString(), tr("Write a name for this image filter list."), QLineEdit::Normal,
                                 QString(), &ok);
    if (!ok)
        return false;

    description = QInputDialog::getText(this, QString(), tr("Write a description for this image filter list."),
                                        QLineEdit::Normal, QString(), &ok);
    if (!ok)
        return false;

    fileName = getSaveFileName(this, "imagefilterlists", tr("IBP Image Filter List (*.ifl);;") +
                                                         tr("All Files (*)"));

    if (fileName.isEmpty())
        return false;

    mViewEditImageFilterList.setName(name);
    mViewEditImageFilterList.setDescription(description);
    if (!mViewEditImageFilterList.save(fileName))
    {
        QMessageBox::information(this, QString(), tr("An error has happen while saving the file to disk."));
        return false;
    }

    mViewEditImageFilterListIsDirty = false;
    return true;
}

void MainWindow::on_mToolbarEditButtonVSplitter_toggled(bool checked)
{
    if (!checked)
        return;
    ui->mViewEditSplitterPreview->setOrientation(Qt::Vertical);
}

void MainWindow::on_mToolbarEditButtonHSplitter_toggled(bool checked)
{
    if (!checked)
        return;
    ui->mViewEditSplitterPreview->setOrientation(Qt::Horizontal);
}

void MainWindow::On_mToolbarEditButtonAddFilterAction_triggered()
{
    QString id = sender()->property("id").toString();
    ImageFilter * f = mMainImageFilterPluginLoader.instantiateFilter(id);
    if (!f)
    {
        return;
    }
    f->setParent(&mViewEditImageFilterList);
    mViewEditImageFilterList.append(f);

    ui->mViewEditWidgetList->append(f->widget(this));
    ui->mViewEditWidgetList->setTitle(ui->mViewEditWidgetList->count() - 1,
                                      f->info().value("name"), f->info().value("description"));
}
