#include "Header/access2records.h"

#ifdef NewUI
    #include "ui_newaccess2records.h"
#else
    #include "ui_access2records.h"
#endif

#include "Common/commonfunction.h"
#include "Common/logicinterface.h"
#include "Header/picturescale.h"
#include "Dialog/dlggarageingarage.h"

CAccess2Records::CAccess2Records(QWidget* mainWnd, QWidget *parent) :
    QFrame(parent),
    ui(
        #ifdef NewUI
            new Ui::CONCAT_NEW_NAME( New , CAccess2Records )
        #else
            new Ui::CONCAT_NEW_NAME( , CAccess2Records )
        #endif
        )
{
    ui->setupUi(this);
    pParent = dynamic_cast< MainWindow* > ( mainWnd );
    CCommonFunction::ConnectCloseButton( ui->lblClose );

    ControlDataGrid( );
    ui->lblTitle->setText( windowTitle( ) );

    lblImage[ 0 ] = ui->lblImg0;
    lblImage[ 1 ] = ui->lblImg1;
    lblImage[ 2 ] = ui->lblImg2;
    lblImage[ 3 ] = ui->lblImg3;
    lblImage[ 4 ] = ui->lblImg4;
    lblImage[ 5 ] = ui->lblImg5;
    lblImage[ 6 ] = ui->lblImg6;
    lblImage[ 7 ] = ui->lblImg7;

    gbImage[ 0 ] = ui->gb0;
    gbImage[ 1 ] = ui->gb1;
    gbImage[ 2 ] = ui->gb2;
    gbImage[ 3 ] = ui->gb3;
    gbImage[ 4 ] = ui->gb4;
    gbImage[ 5 ] = ui->gb5;
    gbImage[ 6 ] = ui->gb6;
    gbImage[ 7 ] = ui->gb7;

    cbxQuery[ 0 ] = ui->cbCardNo;
    cbxQuery[ 1 ] = ui->cb2;
    cbxQuery[ 2 ] = ui->cbInChannel;
    cbxQuery[ 3 ] = ui->cbOutChannel;
    cbxQuery[ 4 ] = ui->cbInPlate;
    cbxQuery[ 5 ] = ui->cbOutPlate;

    QRect rect;
    rect.setX( 10 );
    rect.setY( 20 );
    rect.setWidth( 251 );
    rect.setHeight( 251 );

    for ( int nIndex = 0; nIndex < 8; nIndex++ ) {
        lblImage[ nIndex ]->setVisible( false );
        lblScaleImage[ nIndex ] = new CPictureScale( nIndex, rect,
                                                                              this, gbImage[ nIndex ] );
        connect( lblScaleImage[ nIndex ], SIGNAL( HideCtrl( bool ) ), this, SLOT( HideCtrl( bool ) ) );
    }

    InitQuery( );

    ui->lineEdit->setVisible( false );
    pReportThread = QReportThread::CreateReportThread( );
    connect( pReportThread, SIGNAL( ExecuteSQLData( int,QStringList, int ) ),
             this, SLOT( HandleExecuteSQLData( int, QStringList, int ) ) );

    QHeaderView* pHeader = ui->tableAccessRecord->horizontalHeader( );
    connect( pHeader, SIGNAL(sectionClicked(int)),
             this, SLOT(ColHeaderSectionClicked(int)));
}

void CAccess2Records::ColHeaderSectionClicked(int logicalIndex)
{
    static bool bAsc = true;
    ui->tableAccessRecord->sortByColumn( logicalIndex, bAsc ? Qt::AscendingOrder : Qt::DescendingOrder );
    bAsc = !bAsc;
}

void CAccess2Records::HandleExecuteSQLData( int nType, QStringList lstData, int )
{
    FillTable( lstData );
}

void CAccess2Records::HideCtrl( bool bVisible )
{
    ui->widget->setVisible( bVisible );
}

CAccess2Records::~CAccess2Records()
{
    delete ui;
}

void CAccess2Records::ControlDataGrid( )
{
    int nIndex = 0;
    QHeaderView* pHeader = ui->tableAccessRecord->horizontalHeader( );
    nIndex = 7;
    pHeader->resizeSection( nIndex, pHeader->sectionSize( nIndex ) * 2 );
    nIndex = 9;
    pHeader->resizeSection( nIndex, pHeader->sectionSize( nIndex ) * 2 );
    pHeader->hideSection( ui->tableAccessRecord->columnCount( ) - 1 );
}

void CAccess2Records::closeEvent( QCloseEvent *event )
{
    pParent->ControlChild( event, this );
}

void CAccess2Records::QueryData(QString &strWhere)
{
    QString strSql = QString( "select a.cardno, a.cardkind, a.feenum, a.feefactnum, a.feezkyy, a.cardselfno, \
            a.inshebeiname, a.intime, a.outshebeiname, a.outtime, a.carkind, a.carcp, a.carcpout, a.stoprdid from stoprd a %1" ).arg( strWhere );

    strSql += " order by a.cardno, a.cardselfno, a.intime";
    pReportThread->PostReportEvent( strSql, QMyReportEvent::ExecuteSQL );
}

void CAccess2Records::FillTable( QStringList& lstData )
{
    CCommonFunction::FreeAllRows( ui->tableAccessRecord );
    int nCols = 14;
    int nRows = lstData.size( ) / nCols;

    ui->tableAccessRecord->setRowCount( nRows );

    QStringList lstMonthID;
    QStringList lstSaveID;
    QStringList lstTimeID;
    QString strID;
    QMultiHash<  QString,QTableWidgetItem* > hashItem;

    for ( int nRow = 0; nRow < nRows; nRow++ ) {
        for ( int nCol = 0; nCol < nCols; nCol++ ) {
            QString& strValue = lstData[ nRow * nCols + nCol ];
            QTableWidgetItem* pItem = new QTableWidgetItem( strValue );
            ui->tableAccessRecord->setItem( nRow, nCol, pItem );
            pItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

            switch ( nCol ) {
            case 0 :
                strID = strValue;
                break;

            case 1 :
                if ( "月租卡" == strValue ) {
                    if ( strID.contains( "(" ) )  {
                        strID = strID.left( strID.indexOf( "(" ) );
                    }

                    if ( !lstMonthID.contains("'" + strID + "'" ) ) {
                        lstMonthID << "'" + strID + "'";
                    }
                } else if ( "储值卡" == strValue ) {
                    if ( !lstSaveID.contains( "'" + strID + "'" ) ) {
                        lstSaveID << "'" + strID + "'";
                    }
                } else if ( "计时卡" == strValue ) {
                    if ( !lstTimeID.contains( "'" + strID + "'" ) ) {
                        lstTimeID << "'" + strID + "'";
                    }
                }
                break;

            case 5 :
                hashItem.insertMulti( strID, pItem );
                break;
            }
        }
    }

    if ( 0 < nRows ) {
        ui->btnSerach->setEnabled( true );
        QString strSql = "";

        if ( lstMonthID.count( ) > 0 ) {
            strSql = "Select cardno,cardselfno from monthcard where cardno in(" + lstMonthID.join( "," ) + ") ";
        }

        if ( lstSaveID.count( ) > 0 ) {
            if ( !strSql.isEmpty( ) ) {
                strSql += " union ";
            }

            strSql += "Select cardno,cardselfno from savecard where cardno in(" + lstSaveID.join( "," ) + ") ";
        }

        if ( lstTimeID.count( ) > 0 ) {
            if ( !strSql.isEmpty( ) ) {
                strSql += " union ";
            }

            strSql += "Select cardno,cardselfno from tmpcard where cardno in(" + lstTimeID.join( "," ) + ") ";
        }

        lstData.clear( );
        nRows = CLogicInterface::GetInterface( )->ExecuteSql( strSql, lstData );

        for ( int nRow = 0; nRow < nRows; nRow++ ) {
            QString& strValue = lstData[ nRow * 2 + 0 ];
            QList< QTableWidgetItem* > lstItem = hashItem.values( strValue );

            foreach ( QTableWidgetItem* item,  lstItem ) {
                 item->setText( lstData[ nRow * 2 + 1 ] );
            }
        }
    }
}

void CAccess2Records::FillTable( QString& strWhere )
{
    CCommonFunction::FreeAllRows( ui->tableAccessRecord );

    QStringList lstData;
    int nCols = 14;
    int nRows; // = CLogicInterface::GetInterface( )->OperateOutInInfo( lstData, CommonDataType::SelectData, strWhere );

    QString strSql = QString( "select a.cardno, a.cardkind, a.feenum, a.feefactnum, a.feezkyy, a.cardselfno, \
            a.inshebeiname, a.intime, a.outshebeiname, a.outtime, a.carkind, a.carcp, a.carcpout, a.stoprdid from stoprd a %1" ).arg( strWhere );

    strSql += " order by a.cardno, a.cardselfno, a.intime";

    nRows = CLogicInterface::GetInterface( )->ExecuteSql( strSql, lstData, CCommonFunction::GetHistoryDb( ) );

    ui->tableAccessRecord->setRowCount( nRows );

    QStringList lstMonthID;
    QStringList lstSaveID;
    QStringList lstTimeID;
    QString strID;
    QMultiHash<  QString,QTableWidgetItem* > hashItem;

    for ( int nRow = 0; nRow < nRows; nRow++ ) {
        for ( int nCol = 0; nCol < nCols; nCol++ ) {
            QString& strValue = lstData[ nRow * nCols + nCol ];
            QTableWidgetItem* pItem = new QTableWidgetItem( strValue );
            ui->tableAccessRecord->setItem( nRow, nCol, pItem );
            pItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

            switch ( nCol ) {
            case 0 :
                strID = strValue;
                break;

            case 1 :
                if ( "月租卡" == strValue ) {
                    if ( strID.contains( "(" ) )  {
                        strID = strID.left( strID.indexOf( "(" ) );
                    }

                    if ( !lstMonthID.contains("'" + strID + "'" ) ) {
                        lstMonthID << "'" + strID + "'";
                    }
                } else if ( "储值卡" == strValue ) {
                    if ( !lstSaveID.contains( "'" + strID + "'" ) ) {
                        lstSaveID << "'" + strID + "'";
                    }
                } else if ( "计时卡" == strValue ) {
                    if ( !lstTimeID.contains( "'" + strID + "'" ) ) {
                        lstTimeID << "'" + strID + "'";
                    }
                }
                break;

            case 5 :
                hashItem.insertMulti( strID, pItem );
                break;
            }
        }
    }

    if ( 0 < nRows ) {
        ui->btnSerach->setEnabled( true );
        strSql = "";

        if ( lstMonthID.count( ) > 0 ) {
            strSql = "Select cardno,cardselfno from monthcard where cardno in(" + lstMonthID.join( "," ) + ") ";
        }

        if ( lstSaveID.count( ) > 0 ) {
            if ( !strSql.isEmpty( ) ) {
                strSql += " union ";
            }

            strSql += "Select cardno,cardselfno from savecard where cardno in(" + lstSaveID.join( "," ) + ") ";
        }

        if ( lstTimeID.count( ) > 0 ) {
            if ( !strSql.isEmpty( ) ) {
                strSql += " union ";
            }

            strSql += "Select cardno,cardselfno from tmpcard where cardno in(" + lstTimeID.join( "," ) + ") ";
        }

        lstData.clear( );
        nRows = CLogicInterface::GetInterface( )->ExecuteSql( strSql, lstData );

        for ( int nRow = 0; nRow < nRows; nRow++ ) {
            QString& strValue = lstData[ nRow * 2 + 0 ];
            QList< QTableWidgetItem* > lstItem = hashItem.values( strValue );

            foreach ( QTableWidgetItem* item,  lstItem ) {
                 item->setText( lstData[ nRow * 2 + 1 ] );
            }
        }
    }
}

void CAccess2Records::resizeEvent( QResizeEvent* )
{
    //FillTable( );
    CalculatePos( );
}

void CAccess2Records::showEvent(QShowEvent *)
{
    pParent->SetCardControl( ui->lineEdit );
    //FillTable( );
}

void CAccess2Records::hideEvent( QHideEvent* )
{
    pParent->SetCardControl( NULL );
}

void CAccess2Records::CalculatePos( )
{
    move( 23, 55 );
    return;
    QRect rectWnd = this->geometry( );

    QString strName = "widget";
    QWidget* pWg = findChild< QWidget* >( strName );
    QRect rectWg = pWg->geometry( );
    pWg->setGeometry( ( rectWnd.width( ) - rectWg.width( ) -2 ) / 2,
                      ( rectWnd.height( )  - rectWg.height( ) -2 ) / 2,
                      rectWg.width( ), rectWg.height( ) );
}

void CAccess2Records::on_btnMinimalize_clicked()
{
    pParent->SetCardControl( NULL );
    setVisible( false );
    pParent->ControlMonitor( true );
}

void CAccess2Records::GetImage( CommonDataType::BlobType blob, int nRow, bool bEnter )
{

    QSettings* pSystemSet = CCommonFunction::GetSettings( CommonDataType::CfgSystem );
    pSystemSet->sync( );
    QString strWhere;

    //bool bSenseOpenGate = pSystemSet->value( "CommonCfg/SenseOpenGate", false ).toBool( );
    //if ( bSenseOpenGate ) {
    //    strWhere = QString( " Where cardno = '%1'" ).arg( ui->tableAccessRecord->item( nRow, 0 )->text( ) );
    //} else {
        QString strCardno = ui->tableAccessRecord->item( nRow, ui->tableAccessRecord->columnCount( ) - 1 )->text( );

        strWhere = QString( " Where stoprdid = '%1'" ).arg(
                                strCardno );
    //}

    QLabel* pLbl = lblScaleImage[ blob ];

    CCommonFunction::LoadFourImages( blob, strWhere, *pLbl, CCommonFunction::GetHistoryDb( ) );
}

void CAccess2Records::on_tableAccessRecord_cellClicked(int row, int column)
{
    for ( int nIndex = 0; nIndex < 8; nIndex++ ) {
        GetImage( ( CommonDataType::BlobType ) nIndex, row, 4 > nIndex );
    }
}

void CAccess2Records::on_lblClose_linkActivated(QString)
{
    close( );
}

void CAccess2Records::on_tableAccessRecord_cellDoubleClicked(int row, int column )
{
    QTableWidget* tab = ui->tableAccessRecord;
    QString strImagePath;
    CCommonFunction::GetPath( strImagePath, CommonDataType::PathUIImage );
    strImagePath +="NewIcon/CommonMiddleBG-normal.jpg";

    QString strWhere = QString( " Where CardId = '%1' and DateTime between '%2' and '%3' " ).arg(
                tab->item( row, 0 )->text( ),
                tab->item( row, 7 )->text( ),
                tab->item( row, 9 )->text( ) );

    //QString strWhere = QString( " Where CardId = '%1' " ).arg(
    //            tab->item( row, 0 )->text( ) );

    CDlgGarageInGarage dlg;
    dlg.setStatusTip( strImagePath );
    if ( dlg.FillTable( strWhere ) ) {
        dlg.exec( );
    } else {
        CCommonFunction::MsgBox( NULL, "提示", "此车辆无子库记录。", QMessageBox::Information );
    }
}

void CAccess2Records::GetWhere( QString &strWhere )
{
    strWhere = "";

    if ( ui->chkStart->isChecked( ) ) {
        strWhere = ui->dtStart0->statusTip( ).arg( ui->dtStart0->text( ),
                                                   ui->dtStart1->text( ) );
    }

    if ( ui->chkEnd->isChecked( ) ) {
        if ( !strWhere.isEmpty( ) ) {
            strWhere += " AND ";
        }

        strWhere += ui->dtEnd0->statusTip( ).arg( ui->dtEnd0->text( ),
                                                   ui->dtEnd1->text( ) );
    }

    if ( 0 != ui->cbInChannel->currentIndex( ) ) {
        if ( !strWhere.isEmpty( ) ) {
            strWhere += " AND ";
        }

        strWhere += ui->cbInChannel->statusTip( ).arg( ui->cbInChannel->currentText( ) );
    }

    if ( 0 != ui->cbOutChannel->currentIndex( ) ) {
        if ( !strWhere.isEmpty( ) ) {
            strWhere += " AND ";
        }

        strWhere += ui->cbOutChannel->statusTip( ).arg( ui->cbOutChannel->currentText( ) );
    }

    if ( !ui->cbCardNo->currentText( ).isEmpty( ) ) {
        if ( !strWhere.isEmpty( ) ) {
            strWhere += " AND ";
        }

        strWhere += ui->cbCardNo->statusTip( ).arg( ui->cbCardNo->currentText( ) );
    }

    if ( !ui->cbInPlate->currentText( ).isEmpty( ) ) {
        if ( !strWhere.isEmpty( ) ) {
            strWhere += " AND ";
        }

        strWhere += ui->cbInPlate->statusTip( ).arg( ui->cbInPlate->currentText( ) );
    }

    if ( !ui->cbOutPlate->currentText( ).isEmpty( ) ) {
        if ( !strWhere.isEmpty( ) ) {
            strWhere += " AND ";
        }

        strWhere += ui->cbOutPlate->statusTip( ).arg( ui->cbOutPlate->currentText( ) );
    }

    if ( !strWhere.isEmpty( ) ) {
        strWhere = " Where " + strWhere;
    }
}

void CAccess2Records::on_btnQuery_clicked()
{
    QString strWhere = "";
    GetWhere( strWhere );

    if ( strWhere.isEmpty( ) ) {
        CCommonFunction::MsgBox( NULL, "提示", "请输入查询条件！", QMessageBox::Information );
        return;
    }

    //FillTable( strWhere );
    QueryData( strWhere );
}

void CAccess2Records::InitQuery( )
{
    ui->label_2->setVisible( false );
    ui->cb1->setVisible( false );

    QDateTime dt = QDateTime::currentDateTime( );
    dt.setTime( QTime( 0, 0, 0 ));
    ui->dtStart0->setDateTime( dt );
    ui->dtEnd0->setDateTime( ui->dtStart0->dateTime( ) );

    dt.setTime( QTime( 23, 59, 59 ) );
    ui->dtStart1->setDateTime( dt );
    ui->dtEnd1->setDateTime( ui->dtStart1->dateTime( ) );

    ui->cbInChannel->addItem( "全部" );
    ui->cbOutChannel->addItem( "全部" );
    QString strSql = QString( "Select shebeiadr, shebeiname from roadconerinfo" );
    QStringList lstRows;
    int nRows = CLogicInterface::GetInterface( )->ExecuteSql( strSql, lstRows );

    for ( int nRow = 0; nRow < nRows; nRow++ ) {
        QString& strText = lstRows[ nRow * 2 + 1 ];
        if ( 0 == ( lstRows[ nRow * 2 + 0 ].toInt( ) % 2 ) ) {
            ui->cbOutChannel->addItem( strText );
        } else {
            ui->cbInChannel->addItem( strText );
        }
    }

    //ui->label_3->setVisible( false );
    //ui->cb2->setVisible( false );
    return;
    /*
    QDateTime dtCurrent = QDateTime::currentDateTime( );
    dtCurrent = dtCurrent.addMonths( -1 );
    QString strDateTime;
    CCommonFunction::DateTime2String( dtCurrent, strDateTime );

    QString strSql = QString( "select '无' from dual \
            union all \
            select distinct cardno from stoprd  where cardno is not null \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct cardkind from stoprd where cardkind is not null \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct cardselfno from stoprd where cardselfno is not null \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct inshebeiname from stoprd where inshebeiname is not null \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct outshebeiname from stoprd where outshebeiname is not null \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct intime from stoprd where intime >= '%1' \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct outtime from stoprd where outtime  >= '%2' \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct carcp from stoprd where carcp is not null \
            union all \
            select 'end' from dual \
            union all \
            select '无' from dual \
            union all \
            select distinct carcpout from stoprd where carcpout is not null" ).arg( strDateTime, strDateTime );


        QStringList lstRows;
        CLogicInterface::GetInterface( )->ExecuteSql( strSql, lstRows );

        QString strSeperator = ",";
        QString strEnd = ",end,";
        QString strRows = lstRows.join( strSeperator );

        QStringList lstValues = strRows.split( strEnd );
        QStringList lstTmp;

        for ( uint nSection = 0; nSection < sizeof ( cbxQuery ) / sizeof ( QComboBox* ); nSection++ ) {
            lstTmp = lstValues.at( nSection ).split( strSeperator );
            cbxQuery[ nSection ]->addItems( lstTmp );
        }
        */
}

bool CAccess2Records::Search( QComboBox *pCB, int nRow, int nCol )
{
    bool bRet = true;
    QString strText = pCB->currentText( );

    if ( !strText.isEmpty( ) && "全部"  != strText ) {
        QString strRaw = ui->tableAccessRecord->item( nRow, nCol )->text( );
        bRet = strRaw.contains( strText );
    }

    return bRet;
}

void CAccess2Records::on_btnSerach_clicked()
{
    QString strWhere = "";
    GetWhere( strWhere );

    if ( strWhere.isEmpty( ) ) {
        CCommonFunction::MsgBox( NULL, "提示", "请输入查找条件！", QMessageBox::Information );
        return;
    }

    int nRows = ui->tableAccessRecord->rowCount( );
    bool bFound = true;

    for ( int nRow = 0; nRow < nRows; nRow++ ) {
        bFound = true;

        for ( uint nSection = 0; nSection < sizeof ( cbxQuery ) / sizeof ( QComboBox* ); nSection++ ) {
            int nTmp = 0;
            switch ( nSection ) {
            case 0 :
                nTmp = nSection;
                break;

            case 2 :
                nTmp = nSection + 4;
                break;

            case 3 :
                nTmp = nSection + 5;
                break;

            case 4 :
            case 5 :
                nTmp = nSection + 7;
                break;
            }

            bFound &= Search( cbxQuery[ nSection ], nRow, nTmp );
            if ( !bFound ) {
                continue;
            }
        }

        if ( bFound ) {
            ui->tableAccessRecord->selectRow( nRow );
            break;
        }
    }
}

void CAccess2Records::on_lineEdit_textChanged(const QString &arg1)
{

}

void CAccess2Records::on_chkStart_toggled(bool checked)
{
    ui->dtStart0->setEnabled( checked );
    ui->dtStart1->setEnabled( checked );
}

void CAccess2Records::on_chkEnd_toggled(bool checked)
{
    ui->dtEnd0->setEnabled( checked );
    ui->dtEnd1->setEnabled( checked );
}
