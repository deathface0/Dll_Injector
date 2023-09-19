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

void App::renderUI()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar;

    //std::sort(nombres.begin(), nombres.end());
    //ImGui::ShowDemoWindow();
    
    if (ImGui::Begin("Dear ImGui Demo", (bool*)1, window_flags))
    {
        //Load DLL menu
        if (ImGui::Button("Load DLL")) { strcpy_s(Global::dllPath, selectFile().c_str()); }
        ImGui::SameLine();
        ImGui::InputText("", Global::dllPath, IM_ARRAYSIZE(Global::dllPath));
        ImGui::PushItemWidth(100);
        Global::fixedDllPath.assign(Global::dllPath); replace_all(Global::fixedDllPath, "\\", "\\\\"); //DLL path string manipulation

        ImGui::Text("%s running", std::to_string(Global::procList.size()).c_str());

        //Variables
        static ImGuiTableFlags flags1 = ImGuiTableFlags_Borders | ImGuiTableColumnFlags_WidthFixed;
        bool isHovered, isClicked;

        if (Global::loadList)
        {
            Global::procList = DLLInjector::getProcList();
            Global::loadList = false;
        }

        int rows = Global::procList.size() + 1;

        //Process table
        if (ImGui::BeginTable("process_table", 2, flags1))
        {
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableHeadersRow();
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
                if (isClicked) { DLLInjector::Inject(Global::procList[row].id, Global::fixedDllPath.c_str()); }

                // Column 2
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", Global::procList[row].name.c_str());

                isHovered = ImGui::IsItemHovered();
                if (isHovered) { ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(0.5f, 0.5f, 1.0f, 1.0f)); }
                isClicked = ImGui::IsItemClicked(0);
                if (isClicked) { DLLInjector::Inject(Global::procList[row].id, Global::fixedDllPath.c_str()); }
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }
}
