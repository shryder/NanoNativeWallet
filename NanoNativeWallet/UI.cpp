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

int MAIN_WINDOW_STYLE = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

static bool bDemoWindow = false;

static char walletPassword[MAX_WALLET_PASSWORD_LENGTH] = "";
static int accountIndexInput = 0;

char loginPopupSeed[SEED_SIZE + 1] = "";
char loginPopupPassword[MAX_WALLET_PASSWORD_LENGTH] = "";
char loginPopupPasswordRetype[MAX_WALLET_PASSWORD_LENGTH] = "";
char loginPopupWalletAlias[MAX_WALLET_NAME_LENGTH] = "";

size_t selectedWallet = 0;
size_t selectedAccount = 0;

std::vector<Wallet> gWallets = {};

void clear(char* arr, size_t length) {
    memset(arr, 0, sizeof(char) * length);
}

void TextCenter(std::string text) {
    float font_size = ImGui::GetFontSize() * text.size() / 2;
    ImGui::SameLine(
        ImGui::GetWindowSize().x / 2 -
        font_size + (font_size / 2)
    );

    ImGui::Text(text.c_str());
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

        addWallet(Wallet(std::string(loginPopupWalletAlias), std::vector<byte>(encryptedSeed.begin(), encryptedSeed.end()), generatedIV, std::string(loginPopupSeed)));
        switchToWallet(gWallets.size() - 1);

        clear(loginPopupSeed, SEED_SIZE + 1);
        clear(loginPopupPassword, MAX_WALLET_PASSWORD_LENGTH);
        clear(loginPopupPasswordRetype, MAX_WALLET_PASSWORD_LENGTH);
        clear(loginPopupWalletAlias, MAX_WALLET_NAME_LENGTH);
    }

    ImGui::EndChild();
    ImGui::EndGroup();
}

static char newWalletName[MAX_WALLET_NAME_LENGTH] = "";
static char newWalletPassword[MAX_WALLET_PASSWORD_LENGTH] = "";
static char newWalletPasswordRetype[MAX_WALLET_PASSWORD_LENGTH] = "";

void SettingsTab() {
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
        }
        else {
            // MessageBoxA(hwnd, "Passwords do not match", "Error", MB_ICONERROR);
            printf("ERROR: Password do not match");
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Lock Wallet")) {
        getSelectedWallet().lock();
    }

}

void WalletPage() {
    ImGui::BeginGroup();
    ImGui::BeginChild("WalletPage", ImVec2(0, 0));

    if (getSelectedWallet().isEncrypted) {
        ImGui::Text("Unlock Wallet - %s", getSelectedWallet().name.c_str());
        ImGui::Separator();

        bool submitted = ImGui::InputText("##WalletPagePassword", walletPassword, MAX_WALLET_PASSWORD_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue);
        if (submitted || ImGui::Button("Unlock")) {
            getSelectedWallet().unlock(walletPassword);
            clear(walletPassword, MAX_WALLET_PASSWORD_LENGTH);
        }
    } else {
        ImGui::Text("Accounts List - %s", getSelectedWallet().seed.c_str());

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
                    ImGui::BeginChild("AccountsList", ImVec2(800, 0), true);

                    static ImGuiTextFilter filter;
                    filter.Draw("##AccountsListFilterAddresses", 800);

                    ImGui::Spacing();

                    for (std::vector<Account>::size_type i = 0; i < getSelectedWallet().accounts.size(); i++) {
                        Account account = getAccount(i);

                        if (!account.hidden) {
                            std::string row_text = account.address + " - #" + std::to_string(account.index) + "\n" + std::to_string(account.getNANOBalance()) + std::string(" NANO") + "##" + std::to_string(account.index);

                            if (filter.PassFilter(row_text.c_str()) && ImGui::Selectable(row_text.c_str(), selectedAccount == i)) {
                                selectedAccount = i;
                            }
                        }
                    }

                    ImGui::EndChild();
                }

                ImGui::SameLine();

                {
                    ImGui::BeginChild("AccountInfo", ImVec2(350, 0), true);

                    auto account = getAccount(selectedAccount);

                    if (ImGui::Button("Copy Address")) {
                        ImGui::SetClipboardText(account.address.c_str());
                    }

                    ImGui::Text("Account #%d", account.index);
                    ImGui::Text("Balance: %lf NANO", account.getNANOBalance());
                    ImGui::Text("Recent Transactions:");

                    if (account.isAccountOpen) {
                        ImGui::Columns(4, "RecentTransactions");
                        ImGui::Separator();
                        ImGui::Text("Type"); ImGui::NextColumn();
                        ImGui::Text("Account"); ImGui::NextColumn();
                        ImGui::Text("Amount"); ImGui::NextColumn();
                        ImGui::Text("Hash"); ImGui::NextColumn();
                        ImGui::Separator();

                        static int selected = -1;

                        for (int i = 0; i < getAccount(selectedAccount).account_history.size(); i++) {
                            auto transaction = getAccount(selectedAccount).account_history.at(i);

                            std::string account = transaction["account"];
                            std::string hash = transaction["hash"];
                            std::string type = transaction["type"];

                            float amount = rawToNano(decode_dec(transaction["amount"]));

                            std::string label(type + "##" + std::to_string(i));
                            if (ImGui::Selectable(label.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
                                selected = i;

                            bool hovered = ImGui::IsItemHovered();

                            ImGui::NextColumn();
                            ImGui::Text(account.c_str());
                            ImGui::NextColumn();
                            ImGui::Text("%lf", amount);
                            ImGui::NextColumn();
                            ImGui::Text(hash.c_str());
                            ImGui::NextColumn();
                        }

                        ImGui::Columns(1);
                    } else {
                        ImGui::Text("Account is not open");
                    }

                    ImGui::EndChild();
                }

                ImGui::EndTabItem();
                ImGui::EndGroup();
            }

            if (ImGui::BeginTabItem("Settings")) {
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
    ImGui::Begin("MainView", NULL, MAIN_WINDOW_STYLE | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
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
        TextCenter(std::to_string(gWallets.size()) + " wallets");

        ImGui::BeginChild("SideBar", ImVec2(200, 0), true);

        for (std::vector<Wallet>::size_type i = 0; i != gWallets.size(); i++) {
            std::string full_name = gWallets[i].name + "##wallet" + std::to_string(i);

            if (ImGui::Selectable((char*)full_name.c_str(), selectedWallet == i && !showCreateWalletPage)) {
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
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ io.DisplaySize.x, io.DisplaySize.y });

    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;

    if (ImGui::IsKeyReleased(ImGui::GetIO().KeyShift)) {
        bDemoWindow = !bDemoWindow;
    }

    if (!bDemoWindow) {
        MainView();
    } else {
        ImGui::ShowDemoWindow();
    }
}