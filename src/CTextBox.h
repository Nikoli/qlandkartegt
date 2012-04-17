/**********************************************************************************************
    Copyright (C) 2007 Oliver Eichler oliver.eichler@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

**********************************************************************************************/

#ifndef CTEXTBOX_H
#define CTEXTBOX_H

#include <QDialog>

#include "ui_ITextBox.h"

/// dialog to display simple text
class CTextBox : public QDialog, private Ui::ITextBox
{
    Q_OBJECT;
    public:
        CTextBox();
        virtual ~CTextBox();
        void setPlainText(const QString& text) { textEdit->setPlainText(text); };
        void setHtml(const QString& text) { textEdit->setHtml(text); };

    private slots:
        void print();
};
#endif
