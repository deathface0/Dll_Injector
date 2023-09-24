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

GLuint App::CreateTextureFromHICON(HICON& hIcon)
{
    if (hIcon == NULL) {
        return 0;
    }

    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) {
        return 0;
    }
    
    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Obtener la información de la imagen del icono
    BITMAP bmp;
    if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp)) {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return 0;
    }

    // Copiar el icono al contexto de dispositivo
    HDC dc = CreateCompatibleDC(NULL);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(dc, iconInfo.hbmColor);

    // Configurar el formato de píxeles adecuado para OpenGL
    BITMAPINFO bi;
    memset(&bi, 0, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bmp.bmWidth;
    bi.bmiHeader.biHeight = -bmp.bmHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    // Copiar los datos del icono a la textura OpenGL
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmp.bmWidth, bmp.bmHeight, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    // Restaurar el objeto de bitmap original y liberar recursos
    SelectObject(dc, hOldBitmap);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    DeleteDC(dc);

    return textureID;
}

bool App::getProcessIcon(DWORD processId, HICON& phiconLarge, HICON& phiconSmall) {
    // Obtener un identificador de proceso
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        return false;
    }

    // Obtener la ruta del archivo ejecutable del proceso
    TCHAR exePath[MAX_PATH];
    if (GetProcessImageFileName(hProcess, exePath, MAX_PATH) == 0) {
        CloseHandle(hProcess);
        return false;
    }
    std::string formatedExePath = exePath;
    replace_all(formatedExePath, "\\Device\\HarddiskVolume5\\", "C:\\"); // DLL path string manipulation
    replace_all(formatedExePath, "\\", "\\\\"); // DLL path string manipulation
    //std::cout << formatedExePath << std::endl;

    int iconIndex = 0; // Índice del ícono dentro del archivo ejecutable (generalmente 0 para el primer ícono)

    HICON iconLarge;
    HICON iconSmall;

    UINT result = ExtractIconExA(formatedExePath.c_str(), iconIndex, &iconLarge, &iconSmall, 1);

    if (result > 0) {
        std::cout << "HICON OBTAINED" << std::endl;

        phiconLarge = iconLarge;
        phiconSmall = iconSmall;
        
        CloseHandle(hProcess);
        return true;
    }

    DWORD error = GetLastError();
    std::cout << "No se pudo obtener el icono. Código de error: " << error << std::endl;

    phiconLarge = NULL;
    phiconSmall = NULL;

    CloseHandle(hProcess);
    return false;
}

void App::renderImageWithImGui(const std::vector<unsigned char>& imageData, int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();

    ImTextureID my_tex_id = io.Fonts->TexID;
    //ImTextureID textureId = (ImTextureID)io.Fonts->AddFontTexture();

    float my_tex_w = (float)io.Fonts->TexWidth;
    float my_tex_h = (float)io.Fonts->TexHeight;

    ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right

    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

    //// Copia los datos de la imagen al búfer de la textura de ImGui
    std::memcpy(io.Fonts->TexPixelsAlpha8, imageData.data(), imageData.size());
    
    ImGui::Image(my_tex_id, ImVec2(width, height), uv_min, uv_max, tint_col, border_col);
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
    if (ImGui::BeginTable("process_table", 3, flags1))
    {
        // Set column headers
        ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 40.0f);
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
            HICON phiconLarge = NULL, phiconSmall = NULL;
            if (getProcessIcon(Global::procList[row].id, phiconLarge, phiconSmall))
            {
                std::cout << "IMAGE-RENDER";
                GLuint textureID = CreateTextureFromHICON(phiconLarge);
                ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(textureID)), ImVec2(20, 20)); // Ajusta el tamaño según tus necesidades
                glDeleteTextures(1, &textureID);
            }

            // Column 2
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", Global::procList[row].id);

            isHovered = ImGui::IsItemHovered();
            if (isHovered) { ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(0.5f, 0.5f, 1.0f, 1.0f)); }
            isClicked = ImGui::IsItemClicked(0);
            if (isClicked) { Global::selectedProcess = Global::procList[row]; }

            // Column 3
            ImGui::TableSetColumnIndex(2);
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
    ofn.lpstrFilter = "Dynamic Link Libraries (*.dll)\0*.dll\0All Files (*.*)\0*.*\0"; // Filtros de archivos
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
        const char* items[] = { "Basic Injection", "Reflective DLL Injection", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIIIIII", "JJJJ", "KKKKKKK" };
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

        //TODO: Inject at XX:XX:XX

        // Inject button
        if (ImGui::Button("Inject", { 100, 40 }))
            if (DLLInjector::Inject(Global::selectedProcess.id, Global::fixedDllPath.c_str()) && Global::closeOnInject)
                exit(1);
            
            

        ImGui::End();
    }
}
