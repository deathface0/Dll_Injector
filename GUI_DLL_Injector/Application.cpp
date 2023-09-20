#include "Application.h"

std::string App::replace_all(std::string& str, const std::string& find, const std::string& replace)
{
    size_t pos = 0;
    while ((pos = str.find(find, pos)) != std::string::npos) {
        str.replace(pos, find.length(), replace);
        pos += replace.length();
    }
    return str;
}

void App::loadProcList()
{
    if (Global::loadList)
    {
        if (!strcmp(Global::procFilter, "")) // No filter
        {
            Global::procList = DLLInjector::getProcList(); // Obtaining process list
            Global::loadList = false;
        }
        else { // Using filter
            std::vector<Process> list;
            Global::procList = DLLInjector::getProcList(); // Obtaining process list

            std::string lc_filter, lc_procName;
            for (char c : Global::procFilter) { lc_filter += std::tolower(c); } // Convert filter to lowercase

            for (int i = 0; i < Global::procList.size(); i++) {
                for (char c : Global::procList[i].name) { lc_procName += std::tolower(c); } // Convert process name to lowercase
                if (lc_procName.find(lc_filter.c_str()) != std::string::npos) {
                    list.push_back(Global::procList[i]); // Push match to new vector
                }
                lc_procName.clear();
            }

            Global::procList = list;
            Global::loadList = false;
            lc_filter.clear();
            lc_procName.clear();
        }
    }
}

void App::sortProcList()
{
    if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
    {
        std::vector<Process> lc_procList; std::string lc_procName;

        for (int i = 0; i < Global::procList.size(); i++) {
            for (char c : Global::procList[i].name) { lc_procName += std::tolower(c); } // Convert process name to lowercase
            lc_procList.push_back({ Global::procList[i].id, lc_procName });

            lc_procName.clear();
        }

        if (sorts_specs->Specs->SortDirection == 1)
        {
            std::sort(Global::procList.begin(), Global::procList.end(), [&](const Process& n1, const Process& n2)
                {
                    std::string lc_n1, lc_n2;
                    for (char c : n1.name) { lc_n1 += std::tolower(c); } // Convert process name to lowercase
                    for (char c : n2.name) { lc_n2 += std::tolower(c); } // Convert process name to lowercase

                    if (lc_n1 != lc_n2) {
                        return lc_n1 < lc_n2;
                    }
                    else {
                        return n1.name < n2.name;
                    }
                }
            );
        }
        else if (sorts_specs->Specs->SortDirection == 2)
        {
            std::sort(Global::procList.begin(), Global::procList.end(), [&](const Process& n1, const Process& n2)
                {
                    std::string lc_n1, lc_n2;
                    for (char c : n1.name) { lc_n1 += std::tolower(c); } // Convert process name to lowercase
                    for (char c : n2.name) { lc_n2 += std::tolower(c); } // Convert process name to lowercase

                    if (lc_n1 != lc_n2) {
                        return lc_n1 > lc_n2;
                    }
                    else {
                        return n1.name > n2.name;
                    }
                }
            );
        }
    }
}

void App::loadProcTable()
{
    // Variables
    static ImGuiTableFlags flags1 = ImGuiTableFlags_Borders | ImGuiTableColumnFlags_WidthFixed;
    bool isHovered, isClicked;
    int rows = Global::procList.size() + 1; // Set table rows

    // Process table
    if (ImGui::BeginTable("process_table", 2, flags1))
    {
        // Set column headers
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableHeadersRow();

        // Sort our data if sort specs have been changed!
        sortProcList();

        for (int row = 0; row < rows; row++)
        {
            ImGui::TableNextRow();

            if (row == rows - 1) continue;

            // Column 1
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", Global::procList[row].id);

            isHovered = ImGui::IsItemHovered();
            if (isHovered) { ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(0.5f, 0.5f, 1.0f, 1.0f)); }
            isClicked = ImGui::IsItemClicked(0);
            if (isClicked) { Global::selectedProcess = Global::procList[row]; }

            // Column 2
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", Global::procList[row].name.c_str());

            isHovered = ImGui::IsItemHovered();
            if (isHovered) { ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(0.5f, 0.5f, 1.0f, 1.0f)); }
            isClicked = ImGui::IsItemClicked(0);
            if (isClicked) { Global::selectedProcess = Global::procList[row]; }
        }
        ImGui::EndTable();
    }
}

std::string App::selectFile()
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL; // Ventana principal (o NULL si no es relevante)
    ofn.lpstrFilter = "All Files (*.*)\0*.*\0"; // Filtros de archivos
    ofn.lpstrFile = szFileName; // Almacena la ruta del archivo seleccionado
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    // Abre el cuadro de diálogo de selección de archivos
    if (GetOpenFileName(&ofn) == TRUE) {
        // szFileName ahora contiene la ruta del archivo seleccionado
        std::string filePath = szFileName;
        return filePath.c_str();
    }
    return "";
}

void App::renderUI(const OpenGL& window)
{
    //ImGui::ShowDemoWindow();

    renderProcWnd(window);
    renderInjectionWnd(window);    
}

void App::renderProcWnd(const OpenGL& window)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    // Window configuration
    ImGui::SetNextWindowSize(ImVec2(Global::GL_WIN_WIDTH / 2.0f, Global::GL_WIN_HEIGHT));
    ImGui::SetNextWindowPos(ImVec2(window.windowX, window.windowY));
    // Process List Window
    if (ImGui::Begin("Process List Window", (bool*)1, window_flags))
    {
        std::string procView = Global::selectedProcess.name + "(" + std::to_string(Global::selectedProcess.id) + ")";
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), procView.c_str());
        ImGui::Separator();

        // Process filter
        ImGui::Text("Filter");
        ImGui::SameLine();
        ImGui::PushItemWidth(600.0f);
        ImGui::InputText("", Global::procFilter, IM_ARRAYSIZE(Global::procFilter));

        ImGui::Text("%s running", std::to_string(Global::procList.size()).c_str()); // Running processes

        loadProcList();
        loadProcTable();

        ImGui::End();
    }
}

void App::watchProcess()
{
    if (Global::procWatching)
    {
        for (const auto& process : Global::procList)
        {
            if (process.name == Global::procWatched)
            {
                if (DLLInjector::Inject(process.id, Global::fixedDllPath.c_str()) && Global::closeOnInject)
                    exit(1);
                    
                strcpy_s(Global::procWatched, ""); // Clear process watched
                Global::procWatching = false;
            }
        }
    }
}

void App::renderInjectionWnd(const OpenGL& window)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    // Window configuration
    ImGui::SetNextWindowSize(ImVec2(Global::GL_WIN_WIDTH / 2.0f, Global::GL_WIN_HEIGHT));
    ImGui::SetNextWindowPos(ImVec2(window.windowX + Global::GL_WIN_WIDTH / 2.0f, window.windowY));
    // Injection Window
    if (ImGui::Begin("Injection Window", (bool*)1, window_flags))
    {
        // Window configuration
        ImGui::SetNextWindowSize(ImVec2(Global::GL_WIN_WIDTH / 2.0f, Global::GL_WIN_HEIGHT));
        ImGui::SetNextWindowPos(ImVec2(window.windowX, window.windowY));

        // Load DLL menu
        if (ImGui::Button("Load DLL")) { strcpy_s(Global::dllPath, selectFile().c_str()); }
        ImGui::SameLine(0, 44);
        ImGui::PushItemWidth(400.0f);
        ImGui::InputText("DLL", Global::dllPath, IM_ARRAYSIZE(Global::dllPath));
        Global::fixedDllPath.assign(Global::dllPath); replace_all(Global::fixedDllPath, "\\", "\\\\"); // DLL path string manipulation

        // Injection type
        const char* items[] = { "Remote Thread Injection (RTI)", "Reflective DLL Injection", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
        static int item_current = 0;
        ImGui::PushItemWidth(388.0f);
        ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items));

        // Process watcher
        ImGui::Checkbox("Wait Launch", &Global::procWatching);
        ImGui::SameLine();
        ImGui::PushItemWidth(400.0f);
        ImGui::InputText("Watch", Global::procWatched, IM_ARRAYSIZE(Global::procWatched));
        watchProcess();
        
        // Close on inject
        ImGui::Checkbox("Close On Inject", &Global::closeOnInject);

        ImGui::NewLine(); // New
        ImGui::NewLine(); // lines

        // Inject button
        if (ImGui::Button("Inject", { 100, 40 }))
            if (DLLInjector::Inject(Global::selectedProcess.id, Global::fixedDllPath.c_str()))
                exit(1);
            
            

        ImGui::End();
    }
}
