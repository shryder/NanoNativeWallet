#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_style.h"

#include "Models/Wallet.h"
#include "Models/Account.h"

#include "UI.h"
#include "Database/database.h"
#include "NanoNativeWallet.h"
#include "Crypto/crypto_utils.h"
#include "NodeRPC/NodeRPC.h"

const int MAIN_WINDOW_STYLE = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

bool bDemoWindow = false;

size_t selectedWallet = 0;
size_t selectedAccount = 0;

std::vector<Wallet> gWallets = {};

void clear(char* arr, size_t length) {
    memset(arr, 0, sizeof(char) * length);
}

Wallet& getSelectedWallet() {
    return gWallets.at(selectedWallet);
}

Account& getAccount(size_t i) {
    return getSelectedWallet().accounts.at(i);
}

void addWallet(Wallet wallet) {
    gWallets.push_back(wallet);
    saveDatabase();
}

bool showCreateWalletPage = false;

void switchToWallet(size_t index) {
    showCreateWalletPage = false;
    selectedWallet = index;
}

void ImportWalletPage() {
    static char loginPopupSeed[SEED_SIZE + 1] = "";
    static char loginPopupPassword[MAX_WALLET_PASSWORD_LENGTH] = "";
    static char loginPopupPasswordRetype[MAX_WALLET_PASSWORD_LENGTH] = "";
    static char loginPopupWalletAlias[MAX_WALLET_NAME_LENGTH] = "";

    ImGui::BeginGroup();
    ImGui::BeginChild("CreateNewWalletPage", ImVec2(0, 0));

    ImGui::Text("Wallet Alias"); ImGui::SameLine();
    bool submitted = ImGui::InputText("##LoginPopupWalletAlias", loginPopupWalletAlias, MAX_WALLET_NAME_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::Text("Enter your NANO Seed"); ImGui::SameLine();
    submitted = ImGui::InputText("##LoginPopupNanoSeed", loginPopupSeed, SEED_SIZE + 1, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::Text("Enter a password"); ImGui::SameLine();
    submitted = ImGui::InputText("##LoginPopupPassword", loginPopupPassword, MAX_WALLET_PASSWORD_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::Text("Confirm password"); ImGui::SameLine();
    submitted = ImGui::InputText("##LoginPopupPasswordRetype", loginPopupPasswordRetype, MAX_WALLET_PASSWORD_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);

    if (submitted || ImGui::Button("Submit")) {
        std::vector<byte> generatedIV = generateIV();
        std::string encryptedSeed = encryptAES(std::string(loginPopupSeed), std::string(loginPopupPassword), generatedIV);
        std::string uuid = generateUUID();

        addWallet(Wallet(uuid, std::string(loginPopupWalletAlias), std::vector<byte>(encryptedSeed.begin(), encryptedSeed.end()), generatedIV, std::string(loginPopupSeed)));
        switchToWallet(gWallets.size() - 1);

        clear(loginPopupSeed, SEED_SIZE + 1);
        clear(loginPopupPassword, MAX_WALLET_PASSWORD_LENGTH);
        clear(loginPopupPasswordRetype, MAX_WALLET_PASSWORD_LENGTH);
        clear(loginPopupWalletAlias, MAX_WALLET_NAME_LENGTH);
    }

    ImGui::EndChild();
    ImGui::EndGroup();
}

void deleteCurrentWallet(){
    deleteWalletFromDisk(getSelectedWallet().uuid);
    gWallets.erase(gWallets.begin() + selectedWallet);
    saveDatabase();
}

void SettingsTab() {
    static char newWalletName[MAX_WALLET_NAME_LENGTH] = "";

    static char oldWalletPassword[MAX_WALLET_NAME_LENGTH] = "";
    static char newWalletPassword[MAX_WALLET_PASSWORD_LENGTH] = "";
    static char newWalletPasswordRetype[MAX_WALLET_PASSWORD_LENGTH] = "";

    bool nameSubmitted = ImGui::InputTextWithHint("##newWalletName", "Enter new wallet name", newWalletName, MAX_WALLET_NAME_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();

    if (nameSubmitted || ImGui::Button("Change Name")) {
        getSelectedWallet().setName(newWalletName);
        clear(newWalletName, MAX_WALLET_NAME_LENGTH);

        saveDatabase();
    }

    ImGui::Separator();

    ImGui::InputTextWithHint("##newWalletPassword", "New Password", newWalletPassword, MAX_WALLET_PASSWORD_LENGTH);
    bool passwordSubmitted = ImGui::InputTextWithHint("##newWalletPasswordRetype", "Confirm Password", newWalletPasswordRetype, MAX_WALLET_PASSWORD_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SameLine();

    if (passwordSubmitted || ImGui::Button("Change Password")) {
        if (strcmp(newWalletPassword, newWalletPasswordRetype) == 0) {
            getSelectedWallet().updatePassword(newWalletPassword);
            clear(newWalletPassword, MAX_WALLET_PASSWORD_LENGTH);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ERROR: Passwords do not match");
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Lock Wallet")) {
        getSelectedWallet().lock();
    }

    if (ImGui::Button("Delete Wallet")) {
        deleteCurrentWallet();
    }

    if (ImGui::Button("Copy Seed")) {
        ImGui::SetClipboardText(getSelectedWallet().seed.c_str());
    }
}

void ShowPerformanceOverlay () {
    const float DISTANCE = 10.0f;
    int corner = 3;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1)
    {
        window_flags |= ImGuiWindowFlags_NoMove;
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Performance", NULL, window_flags)) {
        ImGui::Text("Test Overlay");
    }
    ImGui::End();
}

void AccountInfo () {
    Account &account = getAccount(selectedAccount);
    
    ImGui::Text(account.address.c_str());

    if (!account.isAccountLoaded) {
        ImGui::Text("Loading...");
        return;
    }

    if (ImGui::Button("Copy Address")) {
        ImGui::SetClipboardText(account.address.c_str());
    }

    ImGui::SameLine();

    if (ImGui::Button ("Reload Data")) {
        account.UpdateAccountInfo();
    }

    ImGui::Text("Account #%d", account.index);
    ImGui::Text("Balance: %s NANO", account.balance_formatted.c_str());
    ImGui::Text("Unclaimed: %s NANO", account.unclaimed_formatted.c_str());
    ImGui::Text("Recent Transactions:");

    if (account.isAccountOpen) {
        ImGui::Columns(4, "RecentTransactions");

        ImGui::Separator();

        ImGui::Text("Type"); ImGui::NextColumn();
        ImGui::Text("Account"); ImGui::NextColumn();
        ImGui::Text("Amount"); ImGui::NextColumn();
        ImGui::Text("Hash"); ImGui::NextColumn();

        // A hack because ImGui's columns api is broke rn
        static bool resizedColumns = false;
        if (!resizedColumns) {
            ImGui::SetColumnWidth(0, 65);
            ImGui::SetColumnWidth(1, 200);
            ImGui::SetColumnWidth(2, 95);
            resizedColumns = true;
        }

        ImGui::Separator();

        static int selected = -1;

        auto account = &getAccount(selectedAccount);
        auto account_history = &account->account_history;

        for (int i = 0; i < account_history->size(); i++) {
            auto transaction = &account_history->at(i);

            // TODO: fix this, allocates a string every frame
            std::string label(transaction->type + "##" + std::to_string(i));
            if (ImGui::Selectable(label.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
                selected = i;

            bool hovered = ImGui::IsItemHovered();

            if (transaction->type == "change") {
                ImGui::NextColumn();
                ImGui::Text(transaction->representative.c_str());
                ImGui::NextColumn();
                ImGui::Text("N/A");
                ImGui::NextColumn();
            } else {
                ImGui::NextColumn();
                ImGui::Text(transaction->account.c_str());
                ImGui::NextColumn();
                ImGui::Text(transaction->amount.c_str());
                ImGui::NextColumn();
            }

            ImGui::Text(transaction->hash.c_str());
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
    } else {
        ImGui::Text("Account is not open");
    }
}

void WalletPage() {
    ImGui::BeginGroup();
    ImGui::BeginChild("WalletPage", ImVec2(0, 0));

    if (getSelectedWallet().isEncrypted) {
        ImGui::Text("Unlock Wallet - %s", getSelectedWallet().name.c_str());
        ImGui::Separator();

        static char walletPassword[MAX_WALLET_PASSWORD_LENGTH] = "";
        bool submitted = ImGui::InputText("##WalletPagePassword", walletPassword, MAX_WALLET_PASSWORD_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);
        if (submitted || ImGui::Button("Unlock")) {
            getSelectedWallet().unlock(walletPassword);
            clear(walletPassword, MAX_WALLET_PASSWORD_LENGTH);
        }
    } else {
        static int accountIndexInput = 0;

        ImGui::Text("Accounts List");

        ImGui::SameLine(ImGui::GetWindowWidth() - 220);
        ImGui::SetNextItemWidth(80);
        ImGui::InputInt("##AccountIndexInput", &accountIndexInput, 0);
        ImGui::SameLine();

        if (ImGui::Button("(+) ADD ACCOUNT")) {
            getSelectedWallet().addAccount(accountIndexInput);
        }

        ImGui::Separator();

        if (ImGui::BeginTabBar("##WalletPageTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Accounts")) {
                ImGui::BeginGroup();
                // Accounts List
                {
                    ImGui::BeginChild("AccountsList", ImVec2(600, 0), true);

                    static ImGuiTextFilter filter;
                    filter.Draw("##AccountsListFilterAddresses", 800);

                    ImGui::Spacing();

                    for (std::vector<Account>::size_type i = 0; i < getSelectedWallet().accounts.size(); i++) {
                        Account &account = getAccount(i);

                        if (!account.hidden) {
                            auto row_text = account.ui_name.c_str();

                            if (filter.PassFilter(row_text) && ImGui::Selectable(row_text, selectedAccount == i)) {
                                selectedAccount = i;
                            }
                        }
                    }

                    ImGui::EndChild();
                }

                ImGui::SameLine();

                {
                    ImGui::BeginChild("AccountInfo", ImVec2(550, 0), true);

                    // Make sure atleast one account is loaded
                    if (getSelectedWallet ().accounts.size () > 0) {
                        AccountInfo();
                    }

                    ImGui::EndChild();
                }

                ImGui::EndTabItem();
                ImGui::EndGroup();
            }

            if (ImGui::BeginTabItem("Wallet Settings")) {
                SettingsTab();

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

    ImGui::EndChild();
    ImGui::EndGroup();
}

void MainView() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ io.DisplaySize.x, io.DisplaySize.y });

    ImGui::Begin("MainView", NULL, MAIN_WINDOW_STYLE | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        ImGui::Text("%d wallets loaded", gWallets.size());

        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Add Wallet", "Ctrl+N")) {
                showCreateWalletPage = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // SideBar 
    {
        ImGui::BeginChild("SideBar", ImVec2(200, 0), true);

        for (std::vector<Wallet>::size_type i = 0; i != gWallets.size(); i++) {
            if (ImGui::Selectable(gWallets[i].ui_name.c_str(), selectedWallet == i && !showCreateWalletPage)) {
                selectedAccount = 0;
                switchToWallet(i);
            }
        }

        if (ImGui::Selectable("Import Wallet", showCreateWalletPage)) {
            showCreateWalletPage = true;
        }

        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Page content
    {
        if (showCreateWalletPage || gWallets.size() == 0) {
            ImportWalletPage();
        }
        else {
            WalletPage();
        }
    }

    ImGui::End();
}

void DrawUI() {
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;

    // Press SPACE to toggle ImGui demo window
    if (ImGui::IsKeyReleased(0x2C)) {
        bDemoWindow = !bDemoWindow;
    }

    if (!bDemoWindow) {
        MainView();
    } else {
        ImGui::ShowDemoWindow();
    }

    ShowPerformanceOverlay();
}