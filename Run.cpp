#include"DxLib.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// 定数
	const int WIDTH = 1024, HEIGHT = 301;

	SetWindowText("バイクゲーム"); // ウィンドウタイトルを設定
	SetGraphMode(WIDTH, HEIGHT, 32); // 画面モードを設定
	ChangeWindowMode(TRUE); // ウィンドウモードに変更
	if (DxLib_Init() == -1) return -1; // DxLibの初期化に失敗した場合は終了
	SetBackgroundColor(0, 0, 0); // 背景色を黒に設定
	SetDrawScreen(DX_SCREEN_BACK); // 描画先を裏画面に設定

	int bgX = 0; // 背景のY座標
	int imgBG = LoadGraph("Image/Bg.png");

	while (1) // メインループ
	{
		// 背景のスクロール処理
		bgX = bgX - 5;
		if (bgX <= -WIDTH) 
			bgX = bgX + WIDTH;

		DrawGraph(bgX, 0, imgBG, FALSE);
		DrawGraph(bgX + WIDTH, 0, imgBG, FALSE);

		ScreenFlip(); // 裏画面と表画面を入れ替える
		WaitTimer(16); // 16ミリ秒待機（約60FPS）
		if (ProcessMessage() == -1) break; // Windowsから情報を受け取りエラーが起きたら終了
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) break; // ESCキーが押されたら終了
	}

	DxLib_End(); // DxLibの後始末
	return 0; // 正常終了
}