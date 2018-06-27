﻿
#include "global.h"
#include "middleSearchWidget.h"

#include<QHeaderView>
#include<QHBoxLayout>
#include<QVBoxLayout>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>
#include<QPainter>
#include<QTime>
#include<QMessageBox>
#include<QMenu>
#include<QScrollBar>

#include"middleLeftStackWidget0.h"
#include"myTablePlayListFinal.h"
#include"myPushButton.h"
#include "mainwindow.h"

#define USE_NETCLOUD 1

loadingWidget::loadingWidget(QString pixurl,int tinypixcount,QString text,QWidget *p)
    :baseWidget(p)
    ,m_text(text)
    ,_index(0)
    , animationtimeline(800)
{
    setAttribute(Qt::WA_TranslucentBackground,true);
    QPixmap pix(pixurl);
    animationtimeline.setFrameRange(0,12);
    animationtimeline.setCurveShape(QTimeLine::LinearCurve);
    connect(&animationtimeline,SIGNAL(frameChanged(int)),SLOT(slot_valuechange(int)));
    animationtimeline.setLoopCount(0);

    for(int i=0;i<tinypixcount;i++)//进入
    {
        m_listpix<<pix.copy(i*(pix.width()/tinypixcount),0,pix.width()/tinypixcount,pix.height());
    }
}
void loadingWidget::paintEvent(QPaintEvent *p)
{
    baseWidget::paintEvent(p);
    QPainter pter(this);
    pter.drawPixmap(width()/2-m_listpix.value(0).width()*2,(height()-m_listpix.value(0).height())/2,m_listpix.at(_index));

    QFont ft("微软雅黑");
    ft.setPixelSize(14);
    QFontMetrics metric(ft);
    pter.setPen(QColor(55,174,254));
    pter.setFont(ft);
    pter.drawText(width()/2-m_listpix.value(0).width()+10,
                  (height()-metric.height())/2,metric.width(m_text),metric.height(),Qt::AlignLeft,m_text);

}

void loadingWidget::showEvent(QShowEvent *e)
{
        baseWidget::showEvent(e);
        if(this->isHidden())
            animationtimeline.stop();
        else
        {
            animationtimeline.start();
            raise();
        }
}
////////////////////////////////////////////////////////////////////////////////
middleSearchWidget::middleSearchWidget(QWidget*p)
    :baseWidget(p)
    , m_table(this)
    ,m_maskwid(":/image/middlewidget/loading.png",12,"正在努力加载中~~",this)
    ,m_btnplay("播放",this)
    ,m_btnadd("添加",this)
    ,m_btndownload("下载",this)
    ,m_checkbox("歌曲名",this)

    ,m_labtext("",this)
    ,m_labelSinger("歌手",this)
    ,m_labelAlbum("专辑",this)
{
    setStyleSheet("QLabel{color:rgb(153,153,153);font-size:12px;}"
                  "QCheckBox{color:rgb(153,153,153);}"
                  "QPushButton{border:1px solid rgb(204,204,204);color:rgb(95,95,95);border-radius:2px;}"
                  "QPushButton:hover{border:1px solid rgb(161,199,238);color:rgb(22,154,243);}");
    m_isRequestFinished=true;

    init();
    initMaskWidget();
}


void middleSearchWidget::initMaskWidget()
{
    m_maskwid.setGeometry(0,1,width(),height());
    m_maskwid.show();
}
void middleSearchWidget::slot_requestSong(const ItemResult &result,SearchStatus status)//返回的chao
{
    if(SearchStatus::Started==status)
    {
        m_songlist.clear();
        m_hashmap.clear();
        int count=m_table.rowCount();
        for(int i=0;i<count;i++)
        {
             m_table.removeRow(0); //不是一个好办法 的没办法
        }
        m_table.clear();
    }
    else if(SearchStatus::Searching==status)
    {

        int row= m_table.rowCount();

        m_songlist.append(result);
        m_hashmap.insert(row,result.strHash);

        m_table.insertRow(row);
        m_table.setItem(row,0,new QTableWidgetItem(""));
        m_table.setItem(row,1,new QTableWidgetItem(result.strMusicName));
        m_table.setItem(row,2,new QTableWidgetItem(result.strSinger));
        m_table.setItem(row,3,new QTableWidgetItem(result.strAlbum));
        m_table.setItem(row,4,new QTableWidgetItem(result.strDur));
        m_table.setItem(row,5,new QTableWidgetItem(""));
    }
    else if(SearchStatus::Finished==status)
    {
        m_maskwid.hide();
    }
}

void middleSearchWidget::resizeEvent(QResizeEvent *e)
{
    baseWidget::resizeEvent(e);
    m_maskwid.setGeometry(0,1,width(),height());

    m_table.horizontalHeader()->resizeSection(1,(m_table.width()-150)/3);
    m_table.horizontalHeader()->resizeSection(2,(m_table.width()-150)/3);
    m_table.horizontalHeader()->resizeSection(3,(m_table.width()-150)/3);

    m_labelSinger.setGeometry(m_table.columnWidth(1)+13+(m_table.columnWidth(2)-m_labelSinger.width())/2+7,
                               62,
                               m_labelSinger.width(),
                               m_labelSinger.height());


    m_labelAlbum.setGeometry(m_table.columnWidth(1)+13+m_table.columnWidth(2)+(m_table.columnWidth(3)-m_labelAlbum.width())/2+7,
                              62,
                              m_labelAlbum.width(),
                              m_labelAlbum.height());
}

void middleSearchWidget::slot_autoRequestNextPage(int value)
{
    if(value==0)
        return;
    if(m_table.verticalScrollBar()->maximum()==value)//scroll to bottom
    {
        if(m_isRequestFinished)
        {
          emit sig_requestSongNextPage();
          setRequestisFinished(false);//prevent to request network for several times;
        }
    }

}
void middleSearchWidget::init()
{
    QHBoxLayout *hlyout=new QHBoxLayout;

    QHBoxLayout *hlyout1=new QHBoxLayout;

    QVBoxLayout *vlyout=new QVBoxLayout;



    m_table.setInitSearchTableWidget(this);


    baseWidget *wid1=new baseWidget(this);
    wid1->setFixedHeight(36);

    QLabel *label=new QLabel("搜索到",this);

    QLabel *label1=new QLabel("的相关歌曲",this);



    m_labtext.setStyleSheet("QLabel{color:red;}");

    m_btnplay.setStyleSheet("QPushButton{border:1px;color:white;background:rgb(22,154,243);border-radius:2px;}"
                             "QPushButton:hover{background:rgb(21,143,225);}");

    m_btnplay.setFixedSize(53,26);
    m_btnadd.setFixedSize(40,26);
    m_btndownload.setFixedSize(40,26);

    hlyout->addWidget(label);
    hlyout->addWidget(&m_labtext);
    hlyout->addWidget(label1);
    hlyout->addStretch();
    hlyout->addWidget(&m_btnplay);
    hlyout->addWidget(&m_btnadd);
    hlyout->addWidget(&m_btndownload);
    wid1->setContentsMargins(0,0,0,0);
    wid1->setLayout(hlyout);

    baseWidget *wid2=new baseWidget(this);
    wid2->setFixedHeight(32);
    wid2->setStyleSheet("QWidget{background:rgb(245,245,245);}");


    QLabel *label4=new QLabel("时长",this);
    QLabel *label5=new QLabel("操作",this);

    m_checkbox.setStyleSheet("QCheckBox::indicator:checked{border-image:url(:/image/middlewidget/checked.png)}"
                             "QCheckBox::indicator:unchecked{border-image:url(:/image/middlewidget/unchecked.png)}");

    m_labelSinger.adjustSize();
    m_labelAlbum.adjustSize();


    m_labelSinger.setAlignment(Qt::AlignCenter);
    m_labelAlbum.setAlignment(Qt::AlignCenter);

    m_labelSinger.raise();
    m_labelAlbum.raise();

    label4->setFixedSize(55,32);
    label5->setFixedSize(85,32);

    label4->setAlignment(Qt::AlignCenter);
    label5->setAlignment(Qt::AlignCenter);

    hlyout1->addWidget(&m_checkbox);
    hlyout1->addStretch(1);
    hlyout1->addWidget(label4);
    hlyout1->addWidget(label5);
    hlyout1->setSpacing(0);
    hlyout1->setContentsMargins(0,0,0,0);
    wid2->setLayout(hlyout1);


    vlyout->addWidget(wid1);
    vlyout->addSpacing(10);
    vlyout->addWidget(wid2);
    vlyout->addWidget(&m_table);
    vlyout->setSpacing(0);
    vlyout->setContentsMargins(5,5,5,0);
    setLayout(vlyout);

    connect(&m_btnadd,SIGNAL(clicked()),this,SLOT(slot_btnaddclicked()));
    connect(&m_btnplay,SIGNAL(clicked()),this,SLOT(slot_btnplayclicked()));
    connect(&m_btndownload,SIGNAL(clicked()),this,SLOT(slot_btndownloadclicked()));
    connect(&m_checkbox,SIGNAL(clicked()),this,SLOT(slot_checkBoxClicked()));

    connect(m_table.verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(slot_autoRequestNextPage(int)));
}
void middleSearchWidget::slot_checkBoxClicked()
{
    if(m_checkbox.checkState()==Qt::Checked)
    {
        for(int i=0;i<m_table.rowCount();i++)
        {
           QCheckBox*box= (QCheckBox*)m_table.cellWidget(i,0);
           box->setCheckState(Qt::Checked);
        }
    }
    else
    {
        for(int i=0;i<m_table.rowCount();i++)
        {
           QCheckBox*box= (QCheckBox*)m_table.cellWidget(i,0);
           box->setCheckState(Qt::Unchecked);
        }
    }
}

void middleSearchWidget::slot_menuWork()
{
    int index= sender()->objectName().toInt();
    myTablePlayListFinal *pFinalTable= mainWindow::GetInstance()->middleStack0()->myTablePlayListFinalVector().at(index);
    int count=m_table.rowCount();
    bool bFind=false;
    for(int i=0;i<count;i++)
    {
         QCheckBox* box= (QCheckBox*)m_table.cellWidget(i,0);
         if(box->checkState()==Qt::Checked)//如果选中项
         {
             bFind=true;
             const ItemResult &result= GetItemByIndex(i);
             mainWindow::GetInstance()->middleStack0()->addMusicToList(result,pFinalTable);
         }
    }
    if(!bFind)
        QMessageBox::information(NULL,"提示","请选择一首歌曲");
}
void middleSearchWidget::slot_btnplayclicked()//obviously,playbutton
{
    int count=m_table.rowCount();
    bool bPlay=true;
    for(int i=0;i<count;i++)
    {
         QCheckBox* box= (QCheckBox*)m_table.cellWidget(i,0);
         if(box->checkState()==Qt::Checked)//如果选中项
         {
             const ItemResult &result= GetItemByIndex(i);
             mainWindow::GetInstance()->middleStack0()->addMusicToDefaultList(result,bPlay);
             bPlay=false;
         }
    }
    if(bPlay)
       QMessageBox::information(NULL,"提示","请选择一首歌曲");
}
void middleSearchWidget::slot_btnaddclicked()
{
    QMenu menu;
    menu.setContentsMargins(4,5,4,10);
    int index=0;
    foreach(myTablePlayListFinal*f,mainWindow::GetInstance()->middleStack0()->myTablePlayListFinalVector())
    {
        QAction *act=new QAction(f->ShowButtonName(),&menu);
        act->setObjectName(QString::number(index));
        connect(act,&QAction::triggered,this,&middleSearchWidget::slot_menuWork);
        menu.addAction(act);
        index++;
    }
    menu.exec(QCursor::pos());

    QList<QAction*> actlist=  menu.actions();
    foreach (QAction* act, actlist) {
        act->deleteLater();
    }
}
void middleSearchWidget::slot_btndownloadclicked()
{

}
