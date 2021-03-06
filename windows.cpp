#include "windows.h"
#include "simulation.h"
#include "plot.h"
#include "wx/treectrl.h"

enum
{
    ID_Tree,
    ID_OptNew,
    ID_OptDel,
    ID_OptRen,
    ID_OptChn,
    ID_SimAdd,
    ID_SimDel,
    ID_SimRen,
    ID_SimChn,
    ID_SimCal,
    ID_SimVis,
    ID_SimSav,
};

BEGIN_EVENT_TABLE(MainWnd, wxFrame)
    EVT_MENU(ID_SimAdd, MainWnd::OnClick)
    EVT_MENU(ID_SimRen, MainWnd::OptRen)
    EVT_MENU(ID_SimDel, MainWnd::SimDel)
    EVT_MENU(ID_SimChn, MainWnd::SimChn)
    EVT_MENU(ID_SimVis, MainWnd::SimVis)
    EVT_MENU(ID_SimCal, MainWnd::SimCal)
    EVT_MENU(ID_SimSav, MainWnd::SimSav)
    EVT_MENU(wxID_NEW, MainWnd::OnClick)
    EVT_MENU(wxID_SAVE, MainWnd::OptSav)
    EVT_MENU(wxID_OPEN, MainWnd::OptLoad)
    EVT_MENU(wxID_EXIT, MainWnd::OnQuit)
    EVT_TREE_SEL_CHANGED(wxID_ANY, MainWnd::ShowInfo)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, MainWnd::PopUp)
    EVT_MENU(ID_OptNew, MainWnd::OnClick)
    EVT_MENU(ID_OptRen, MainWnd::OptRen)
    EVT_MENU(ID_OptChn, MainWnd::OptChn)
    EVT_MENU(ID_OptDel, MainWnd::OptDel)
END_EVENT_TABLE()

WX_DEFINE_OBJARRAY(ArrayOfCaptions);




MainWnd::MainWnd(const wxString &title, const wxPoint &position, const wxSize &size)
    : wxFrame(NULL, -1, title, position, size)
{
    CreateStatusBar();
    SetStatusText(_("Author: Artur Wójcik"));
}

void MainWnd::OnQuit(wxCommandEvent & WXUNUSED(event))
{
    Close(true);
}

void MainWnd::Fill(MainWnd *hWnd)
{

    //Menu creation

    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(wxID_NEW, wxT("New project"), wxT("Create a new project"));
    fileMenu->Append(wxID_OPEN, wxT("Open a project file"), wxT("Open a file with previously saved project..."));
    fileMenu->Append(wxID_SAVE, wxT("Save project"), wxT("Save a created project..."));

    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, wxT("Exit"), wxT("Exit program..."));

    wxMenu *simMenu = new wxMenu;
    simMenu->Append(ID_SimAdd, wxT("Create simulation"), wxT("Create a simulation for chosen project..."));
    simMenu->Append(ID_SimSav, wxT("Save simulation results"), wxT("Save the results of chosen simulation..."));

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("Project"));
    menuBar->Append(simMenu, wxT("Simulation"));
    SetMenuBar(menuBar);

    BoxSizer = new wxBoxSizer(wxHORIZONTAL);
    tree = new wxTreeCtrl(hWnd, ID_Tree, wxDefaultPosition, wxSize(200,400), wxTR_HAS_BUTTONS|wxTR_SINGLE);
    Root = tree->AddRoot(wxT("Options"), 0, 0, new Option(wxT("Options")));

    BoxSizer->Add(tree, 1, wxEXPAND|wxALIGN_LEFT, 0);

    SideSizer = new wxBoxSizer(wxVERTICAL);

    Plot = new CPlot(this);
    SideSizer->Add(Plot->GetPlotPtr(), 4, wxEXPAND, 0);

    SidePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    new wxStaticText(SidePanel, -44, wxEmptyString, wxPoint(5,5), wxDefaultSize, wxALIGN_LEFT);
    new wxStaticText(SidePanel, -45, wxEmptyString, wxPoint(200,5), wxDefaultSize, wxALIGN_LEFT);
    new wxStaticText(SidePanel, -46, wxEmptyString, wxPoint(400,5), wxDefaultSize, wxALIGN_LEFT);
    new wxStaticText(SidePanel, -47, wxEmptyString, wxPoint(5,30), wxDefaultSize, wxALIGN_LEFT);
    new wxStaticText(SidePanel, -48, wxEmptyString, wxPoint(200,30), wxDefaultSize, wxALIGN_LEFT);

    SideSizer->Add(SidePanel, 1, wxEXPAND, 0);
    BoxSizer->Add(SideSizer, 4, wxEXPAND, 0);
    SetSizer(BoxSizer);
    BoxSizer->Fit(this);
    BoxSizer->Layout();
    BoxSizer->SetSizeHints(this);

    Centre();
}



void MainWnd::OnClick(wxCommandEvent &event)
{

    switch(event.GetId())
    {
        case ID_SimAdd:
        {
            wxTreeItemIdValue cookie;
            wxTreeItemId temp;
            SimulationDialog *simDlg = new SimulationDialog(this);
            Simulation *sim;

            if(!tree->GetFirstChild(Root, cookie).IsOk())
            {
                wxMessageBox(wxT("No options have been added!"), wxT("Error"), wxICON_ERROR, this);
                return;

            }
            temp = tree->GetSelection();
            if(!temp.IsOk() || temp == Root)
            {
                wxMessageBox(wxT("You must choose an option to simulate!"), wxT("Error"), wxICON_ERROR, this);
                return;
            }
            if(tree->GetItemParent(temp) != Root)
            {
                wxMessageBox(wxT("You can't simulate a simulation!"), wxT("Error"), wxICON_ERROR, this);
                return;
            }
            if(simDlg->ShowModal() == wxID_OK)
            {
                sim = static_cast<Option*>(tree->GetItemData(temp))->AddSimulation(simDlg->GetGenType(),
                                                       simDlg->GetOptionType(),
                                                       simDlg->GetIterations(),
                                                       simDlg->GetStep());


                temp = tree->AppendItem(temp, simDlg->GetLabel(), 1, 1, sim);
                tree->SetItemTextColour(temp, simDlg->GetColor());
                sim->SetVector(Plot->Draw(this, sim->GetResults(), simDlg->GetColor()));
                sim->GetVector()->SetName(simDlg->GetLabel());
                sim->GetVector()->ShowName(false);
            }
	    delete simDlg;
            break;
        }

        case wxID_NEW:
        case ID_OptNew:
        {
            OptionDialog *optDlg = new OptionDialog(this);
            if(optDlg->ShowModal() == wxID_OK)
            {
                tree->AppendItem(Root, optDlg->GetLabel(), 1, 1,
                                 new Option(optDlg->GetLabel(),
                                           optDlg->GetValue(),
                                           optDlg->GetInterest(),
                                           optDlg->GetRiskFree(),
                                           optDlg->GetVolatility(),
                                           optDlg->GetTime()));
            }
	    delete optDlg;
            break;
        }
    }
}

void MainWnd::ShowInfo(wxTreeEvent &WXUNUSED(event))
{
    wxString temp;
    if(tree->GetSelection() == Root)
    {
        temp = wxT("");
        FindWindow(-44)->SetLabel(temp);
        FindWindow(-45)->SetLabel(temp);
        FindWindow(-46)->SetLabel(temp);
        FindWindow(-47)->SetLabel(temp);
        FindWindow(-48)->SetLabel(temp);
        return;
    }

    Option *tmp = static_cast<Option*>(tree->GetItemData(tree->GetSelection()));

    if(tree->GetItemParent(tree->GetSelection()) != Root)
        tmp = static_cast<Option*>(tree->GetItemData(tree->GetItemParent(tree->GetSelection())));


    temp << wxT("Option value: ") << tmp->GetValue();
    FindWindow(-44)->SetLabel(temp);
    temp = wxT("Interest rate: ");
    temp << tmp->GetInterest();
    FindWindow(-45)->SetLabel(temp);
    temp = wxT("Riskfree rate: ");
    temp << tmp->GetRiskFree();
    FindWindow(-46)->SetLabel(temp);
    temp = wxT("Volatility: ");
    temp << tmp->GetVolatility();
    FindWindow(-47)->SetLabel(temp);
    temp = wxT("Time: ");
    temp << tmp->GetTime();
    FindWindow(-48)->SetLabel(temp);

}
void MainWnd::PopUp(wxTreeEvent &event)
{
    wxTreeItemId item;
    wxMenu *pop;
    item = event.GetItem();
    if(item == Root)
    {
        pop = new wxMenu;
        pop->Append(ID_OptNew, wxT("Add option"), NULL);
        PopupMenu(pop);
        return;
    }

    if(tree->GetItemParent(item) == Root)
    {
        pop = new wxMenu;
        pop->Append(ID_OptNew, wxT("Add option"), NULL);
        pop->AppendSeparator();
        pop->Append(ID_OptDel, wxT("Delete"), NULL);
        pop->Append(ID_OptRen, wxT("Change name"), NULL);
        pop->Append(ID_OptChn, wxT("Change parameters"), NULL);
        pop->AppendSeparator();
        pop->Append(ID_SimAdd, wxT("Add simulation"), NULL);
        PopupMenu(pop);
        return;
    }

    pop = new wxMenu;
    pop->AppendCheckItem(ID_SimVis, wxT("Simulation visible"), wxT(""));
    pop->AppendSeparator();
    if(static_cast<Simulation*>(tree->GetItemData(item))->IsVisible())
        pop->Check(ID_SimVis, true);
    pop->Append(ID_SimDel, wxT("Delete"), NULL);
    pop->Append(ID_SimRen, wxT("Change name"), NULL);
    pop->Append(ID_SimChn, wxT("Change parameters"), NULL);
    pop->Append(ID_SimCal, wxT("Recalculate"), NULL);
    PopupMenu(pop);

    delete pop;
}
void MainWnd::OptRen(wxCommandEvent &WXUNUSED(event))
{
    tree->EditLabel(tree->GetSelection());
}



void MainWnd::SimDel(wxCommandEvent &WXUNUSED(event))
{
    if(wxMessageBox(wxT("Are you sure to delete simulation?"), wxT("Confirm"), wxYES_NO, this) == wxYES)
    {
        mpFXYVector* tmp = static_cast<Simulation*>(tree->GetItemData(tree->GetSelection()))->GetVector();
        Plot->RemoveVector(tmp);
        tree->Delete(tree->GetSelection());
    }
}

void MainWnd::SimChn(wxCommandEvent &WXUNUSED(event))
{
    Simulation *sim = static_cast<Simulation*>(tree->GetItemData(tree->GetSelection()));
    SimulationDialog *simDlg = new SimulationDialog(this,
                                                    wxID_ANY,
                                                    wxT("Change simulation parameters"),
                                                    wxDefaultPosition,
                                                    wxSize(400, 300),
                                                    wxCAPTION|wxSYSTEM_MENU,
                                                    sim->GetGenType(),
                                                    sim->GetOptionType(),
                                                    sim->GetIterations(),
                                                    sim->GetStep(),
                                                    tree->GetItemText(tree->GetSelection()),
                                                    tree->GetItemTextColour(tree->GetSelection()));

    if(simDlg->ShowModal() == wxID_OK)
    {
        sim->setGenType(simDlg->GetGenType());
        sim->setOptionType(simDlg->GetOptionType());
        sim->setIterations(simDlg->GetIterations());
        sim->setStep(simDlg->GetStep());
        tree->SetItemText(tree->GetSelection(), simDlg->GetLabel());
        sim->GetVector()->SetName(simDlg->GetLabel());

        Plot->RemoveVector(sim->GetVector());
        Option *tmp = static_cast<Option*>(tree->GetItemData(tree->GetItemParent(tree->GetSelection())));
        sim->calculate(tmp->GetTime()/365.25, tmp->GetValue(), tmp->GetVolatility(), tmp->GetRiskFree(), tmp->GetInterest());
        sim->SetVector(Plot->Draw(this, sim->GetResults(), simDlg->GetColor()));
        tree->SetItemTextColour(tree->GetSelection(), simDlg->GetColor());
    }
    delete simDlg;
}

void MainWnd::SimVis(wxCommandEvent &event)
{
    Plot->SetVisible(static_cast<Simulation*>(tree->GetItemData(tree->GetSelection()))->GetVector()->GetName(), event);
    static_cast<Simulation*>(tree->GetItemData(tree->GetSelection()))->ToggleVis();
}

void MainWnd::SimCal(wxCommandEvent &WXUNUSED(event))
{
    Simulation *sim = static_cast<Simulation*>(tree->GetItemData(tree->GetSelection()));
    Plot->RemoveVector(sim->GetVector());
    Option *tmp = static_cast<Option*>(tree->GetItemData(tree->GetItemParent(tree->GetSelection())));
    sim->calculate(tmp->GetTime()/365.25, tmp->GetValue(), tmp->GetVolatility(), tmp->GetRiskFree(), tmp->GetInterest());
    sim->SetVector(Plot->Draw(this, sim->GetResults(), tree->GetItemTextColour(tree->GetSelection())));
}

void MainWnd::OptChn(wxCommandEvent &WXUNUSED(event))
{
    Option *opt = static_cast<Option*>(tree->GetItemData(tree->GetSelection()));
    OptionDialog *optDlg = new OptionDialog(this,
                                            wxID_ANY,
                                            wxT("Change option parameters"),
                                            wxDefaultPosition,
                                            wxSize(500, 250),
                                            wxCAPTION|wxSYSTEM_MENU,
                                            opt->GetTime(),
                                            opt->GetValue(),
                                            opt->GetVolatility(),
                                            opt->GetRiskFree(),
                                            opt->GetInterest(),
                                            tree->GetItemText(tree->GetSelection()));

    if(optDlg->ShowModal() == wxID_OK)
    {
        opt->SetTime(optDlg->GetTime());
        opt->SetValue(optDlg->GetValue());
        opt->SetVolatility(optDlg->GetVolatility());
        opt->SetRiskFree(optDlg->GetRiskFree());
        opt->SetInterest(optDlg->GetInterest());
        tree->SetItemText(tree->GetSelection(), optDlg->GetLabel());
        wxTreeEvent evt;
        ShowInfo(evt);

        if(tree->GetChildrenCount(tree->GetSelection(), false) == 0)
            return;

        if(wxMessageBox(wxT("Option parameters has been changed, do you want to recalculate simulations?"), wxT("Question"), wxYES_NO, this) == wxYES)
        {
            wxTreeItemIdValue cookie;
            wxTreeItemId child;
            Simulation *sim;
            Option *tmp = static_cast<Option*>(tree->GetItemData(tree->GetSelection()));

            child = tree->GetFirstChild(tree->GetSelection(), cookie);
            while(child.IsOk())
            {
                sim = static_cast<Simulation*>(tree->GetItemData(child));
                Plot->RemoveVector(sim->GetVector());
                sim->calculate(tmp->GetTime()/365.25, tmp->GetValue(), tmp->GetVolatility(), tmp->GetRiskFree(), tmp->GetInterest());
                sim->SetVector(Plot->Draw(this, sim->GetResults(), tree->GetItemTextColour(child)));
                child = tree->GetNextChild(tree->GetSelection(), cookie);
            }
        }
    }
    delete optDlg;
}

void MainWnd::OptDel(wxCommandEvent &WXUNUSED(event))
{
    if(tree->GetChildrenCount(tree->GetSelection()) > 0)
    {
        if(wxMessageBox(wxT("Are you sure to delete option with its simulations??"), wxT("Confirm"), wxYES_NO, this) != wxYES)
            return;

        wxTreeItemIdValue cookie;
        wxTreeItemId child, tmp;
        Simulation *sim;
        child = tree->GetFirstChild(tree->GetSelection(), cookie);

            while(child.IsOk())
            {
                sim = static_cast<Simulation*>(tree->GetItemData(child));
                Plot->RemoveVector(sim->GetVector());
		tree->Delete(child);
                child = tree->GetFirstChild(tree->GetSelection(), cookie);

            }
    }
    else
    {
        if(wxMessageBox(wxT("Are you sure to delete an option?"), wxT("Confirm"), wxYES_NO, this) != wxYES)
            return;
    }

    tree->Delete(tree->GetSelection());
    tree->SelectItem(Root);
    wxTreeEvent evt;
    ShowInfo(evt);
}

void MainWnd::SimSav(wxCommandEvent &WXUNUSED(event))
{
    if(!tree->GetSelection().IsOk() || tree->GetSelection() == Root)
    {
        wxMessageBox(wxT("Nothing has been chosen"), wxT("Error"), wxICON_ERROR, this);
        return;
    }
        if(tree->GetItemParent(tree->GetSelection()) == Root)
    {
        wxMessageBox(wxT("To save an option choose Save project"), wxT("Error"), wxICON_ERROR, this);
        return;
    }
    wxFileDialog dialog(this, wxT("Save simulation resuls"), wxT(""), tree->GetItemText(tree->GetSelection())+wxT(".dsv"), wxT("DSV (separator : ) (*.dsv)|*.dsv|GNUPlot (*.dat)|*.dat"), wxSAVE|wxOVERWRITE_PROMPT);

    if(dialog.ShowModal() == wxID_OK)
    {
        ofstream file(dialog.GetPath().ToAscii(), ofstream::trunc);
        mCarloResults* results;
        char separator = ':';
        if(dialog.GetFilterIndex()) separator = ' ';

        results = static_cast<Simulation*>(tree->GetItemData(tree->GetSelection()))->GetResults();
        vector<double>::iterator itX, itY;

        for(itX = results->x.begin(), itY = results->y.begin() ; itY < results->y.end() ; itX++, itY++)
        {
            file << *itX << separator << *itY << "\n";
        }
        file.close();

    }
}

void MainWnd::OptSav(wxCommandEvent &WXUNUSED(event))
{
    if(!tree->GetSelection().IsOk() || tree->GetSelection() == Root)
    {
        wxMessageBox(wxT("Nothing has been chosen"), wxT("Error"), wxICON_ERROR, this);
        return;
    }

    if(tree->GetItemParent(tree->GetSelection()) != Root)
    {
        wxMessageBox(wxT("To save simulation, choose Save simulation resuls"), wxT("Error"), wxICON_ERROR, this);
        return;
    }

    wxFileDialog dialog(this, wxT("Save project"), wxT(""), tree->GetItemText(tree->GetSelection())+wxT(".dat"), wxT("Project file (*.dat)|*.dat"), wxSAVE|wxOVERWRITE_PROMPT);

    if(dialog.ShowModal() == wxID_OK)
    {
        
        wxTextFile file(dialog.GetPath());

        if(!file.Create())
        {
            file.Open();
            file.Clear();
        }

        wxTreeItemId child;
        wxTreeItemIdValue cookie;
        vector<double>::iterator itX, itY;
        mCarloResults* results;
        Simulation *sim;
        wxString tmp;

        wxTreeItemId sel = tree->GetSelection();
        Option *opt = static_cast<Option*>(tree->GetItemData(sel));

        file.AddLine(tree->GetItemText(sel));
        tmp << opt->GetValue() << wxT(":") << opt->GetInterest() << wxT(":") << opt->GetRiskFree() << wxT(":") << opt->GetVolatility() << wxT(":") << opt->GetTime();
        file.AddLine(tmp);
        file.Write();
        child = tree->GetFirstChild(sel, cookie);

        while(child.IsOk())
        {
            sim = static_cast<Simulation*>(tree->GetItemData(child));
            file.AddLine(wxT("========"));
            file.AddLine(tree->GetItemText(child));
            tmp = wxT("");
            tmp << sim->GetGenType() << wxT(":") << sim->GetOptionType() << wxT(":") << sim->GetIterations() << wxT(":") << sim->GetStep() << wxT(":") << tree->GetItemTextColour(child).GetAsString();
            file.AddLine(tmp);
            

            results = sim->GetResults();
            tmp = wxT("");
            tmp << results->xmax << wxT(":") << results->ymax;
            file.AddLine(tmp);

            for(itX = results->x.begin(), itY = results->y.begin() ; itX < results->x.end() ; itX++, itY++)
            {
                tmp = wxT("");
                tmp << *itX << wxT(":") << *itY;
                file.AddLine(tmp);
            }
            file.Write();
            child = tree->GetNextChild(sel, cookie);
        }
        file.Close();
    }
}


void MainWnd::OptLoad(wxCommandEvent &WXUNUSED(event))
{
    wxFileDialog dialog(this, wxT("Load project file"), wxT(""), wxT(""), wxT("Project file (*.dat)|*.dat"), wxOPEN);

    if(dialog.ShowModal() == wxID_OK)
    {
        wxTextFile file(dialog.GetPath());
        wxString line, tmp;
        wxTreeItemId optId;
        wxColor color;
        mCarloResults *results;
        Option *opt;
        double val;
        Simulation *sim = NULL;

        file.Open();
        line = file.GetFirstLine();
        opt = new Option(line);
        line = file.GetNextLine();
        tmp = line.SubString(0, line.Find(':'));
        line = line.SubString(line.Find(':')+1, line.Len());
        tmp.ToDouble(&val);
        opt->SetValue(val);

        tmp = line.SubString(0, line.Find(':'));
        line = line.SubString(line.Find(':')+1, line.Len());
        tmp.ToDouble(&val);
        opt->SetInterest(val);

        tmp = line.SubString(0, line.Find(':'));
        line = line.SubString(line.Find(':')+1, line.Len());
        tmp.ToDouble(&val);
        opt->SetRiskFree(val);

        tmp = line.SubString(0, line.Find(':'));
        line = line.SubString(line.Find(':')+1, line.Len());
        tmp.ToDouble(&val);
        opt->SetVolatility(val);

        line = line.SubString(line.Find(':')+1, line.Len());
        line.ToDouble(&val);
        opt->SetTime(val);


        optId = tree->AppendItem(Root, opt->GetDesc(), 1, 1, opt);

        line = file.GetNextLine();
        if(file.Eof())
            return;


        while(!file.Eof())
        {
            if(line.Cmp(wxT("========")) == 0)
            {
                if(sim != NULL)
                {
                    tree->SetItemTextColour(tree->AppendItem(optId, sim->GetDesc(), 1, 1, sim), color);
                    sim->SetResults(results);
                    sim->SetVector(Plot->Draw(this, sim->GetResults(), color));
                    delete results;
                }
                sim = new Simulation();
                results = new mCarloResults;

                line = file.GetNextLine();
                sim->setDesc(line);

                line = file.GetNextLine();

                tmp = line.SubString(0, line.Find(':'));
                line = line.SubString(line.Find(':')+1, line.Len());
                tmp.ToDouble(&val);
                sim->setGenType(val);

                tmp = line.SubString(0, line.Find(':'));
                line = line.SubString(line.Find(':')+1, line.Len());
                tmp.ToDouble(&val);
                sim->setOptionType(val);

                tmp = line.SubString(0, line.Find(':'));
                line = line.SubString(line.Find(':')+1, line.Len());
                tmp.ToDouble(&val);
                sim->setIterations(val);

                tmp = line.SubString(0, line.Find(':'));
                line = line.SubString(line.Find(':')+1, line.Len());
                tmp.ToDouble(&val);
                sim->setStep(val);

                color.Set(line);

                line = file.GetNextLine();
                tmp = line.SubString(0, line.Find(':'));
                line = line.SubString(line.Find(':')+1, line.Len());
                tmp.ToDouble(&val);
                results->xmax = val;
                line.ToDouble(&val);
                results->ymax = val;

                line = file.GetNextLine();
            }

            line.SubString(0, line.Find(':')).ToDouble(&val);
            results->x.push_back(val);

            line.SubString(line.Find(':')+1, line.Len()).ToDouble(&val);
            results->y.push_back(val);

            line = file.GetNextLine();
        }

        tree->SetItemTextColour(tree->AppendItem(optId, sim->GetDesc(), 1, 1, sim), color);
        sim->SetResults(results);
        sim->SetVector(Plot->Draw(this, sim->GetResults(), color));
        delete results;
    }
}
