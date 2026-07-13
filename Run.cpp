#include "DxLib.h"

// バイクの画像を管理する定数と配列
enum { BIKE_RED };
const int BIKE_MAX = 1;
int imgBike[BIKE_MAX];

// バイクの表示位置
void DrawBikeAndHitBox(int x, int y, int type, int* rL, int* rR, int* rT, int* rB, int* fL, int* fR, int* fT, int* fB)
{
	// バイクの絵を描画
	DrawGraph(x, y, imgBike[type], TRUE);

	*rL = x + 5;       // 後輪の左端
	*rR = x + 95;       // 後輪の右端
	*rT = y + 125;      // タイヤのゴム部分
	*rB = y + 155;      // タイヤの接地している一番底辺

	*fL = x + 190;      // 前輪の左端
	*fR = x + 290;      // 前輪の右端
	*fT = y + 125;      // タイヤのゴム部分
	*fB = y + 155;      // タイヤの接地している一番底辺
}

// 2つの円が当たっているか計算する関数
bool CheckCircleHit(int c1x, int c1y, int c1r, int c2x, int c2y, int c2r)
{
	int dx = c1x - c2x;
	int dy = c1y - c2y;
	int distanceSquare = dx * dx + dy * dy;
	int radiusSum = c1r + c2r;
	return distanceSquare < (radiusSum * radiusSum);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	const int WIDTH = 1014, HEIGHT = 768;

	SetWindowText("バイクゲーム");
	SetGraphMode(WIDTH, HEIGHT, 32);
	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1) return -1;
	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);

	int bgX = 0;
	int imgBG = LoadGraph("Image/BG.jpg");
	imgBike[BIKE_RED] = LoadGraph("Image/RedBike.png");

	int imgObstacle = LoadGraph("Image/Rock.png");
	const int OBSTACLE_WIDTH = 130, OBSTACLE_HEIGHT = 130;

	// 追加アイテムの画像を読み込む
	int imgItem = LoadGraph("Image/Item1.png");
	const int ITEM_WIDTH = 50, ITEM_HEIGHT = 50;

	// スコアアップアイテムの画像を読み込む
	int imgScoreUP = LoadGraph("Image/ScoreUP.png");
	const int SCOREUP_WIDTH = 50, SCOREUP_HEIGHT = 50;

	// BGMの読み込み
	int soundBGM = LoadSoundMem("Sound/BGM.mp3");
	PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);

	// 衝突時の効果音の読み込み
	int soundHit = LoadSoundMem("Sound/BikeSE.mp3");

	// アイテムを取得したときの効果音の読み込み
	int soundItem = LoadSoundMem("Sound/Item1SE.mp3");

	// スコアアップアイテムを取得したときの効果音の読み込み
	int soundScoreUP = LoadSoundMem("Sound/ScoreUPSE.mp3");

	// 岩を破壊したときの効果音の読み込み
	int soundDestroy = LoadSoundMem("Sound/DestroySE.mp3");

	// 無敵状態の効果音の読み込み
	int soundInvincible = LoadSoundMem("Sound/InvincibleSE.mp3");

	const int FLOOR_Y = 472;

	int playerX = 100;
	int playerY = FLOOR_Y; // バイクの初期位置を正しい地面にする
	int playerVY = 0;
	int playerType = BIKE_RED;

	int obstacleX = WIDTH;
	// 岩の高さ
	int obstacleY = FLOOR_Y + 20;
	int obstacleSpeed = 15;

	// 追加アイテムの変数
	int itemX = WIDTH + 1500 ;
	int itemY = FLOOR_Y - 100; // アイテムの高さを調整
	int itemSpeed = 15;
	int isItemExit = 1; // アイテムが存在するかどうかのフラグ

	// スコアアップアイテムの変数
	int scoreUpX = WIDTH + 2000;
	int scoreUpY = FLOOR_Y - 80;
	int scoreUpSpeed = 15;
	int isScoreUpExit = 1; // スコアアップアイテムが存在するかどうかのフラグ


	// 無敵状態のタイマー
	int invincibleTimer = 0;

	int isGameOver = 0;
	// スコア
	int score = 0;
	// ハイスコア
	int hiscore = 0;

	// ゲームの状態を管理する変数
	int gameState = 0;

	// タイヤの判定用変数
	int rearLeft = 0, rearRight = 0, rearTop = 0, rearBottom = 0;
	int frontLeft = 0, frontRight = 0, frontTop = 0, frontBottom = 0;

	// メインループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面クリア
		ClearDrawScreen();

		// タイトル画面の処理
		if (gameState == 0)
		{
			// スペースキーが押されたらゲーム開始
			if (CheckHitKey(KEY_INPUT_SPACE) == 1)
			{
				playerX = 100;
				playerY = FLOOR_Y;
				playerVY = 0;
				obstacleX = WIDTH;
				obstacleY = FLOOR_Y + 20;
				itemX = WIDTH + 1500;
				itemY = FLOOR_Y - 100;
				isItemExit = 1;
				scoreUpX = WIDTH + 2000;
				isScoreUpExit = 1;

				invincibleTimer = 0;
				isGameOver = 0;
				score = 0;

				PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
				gameState = 1; // ゲームプレイ状態に遷移
			}
		}
		// ゲームプレイ中の処理
		else if (gameState == 1)
		{
			if (isGameOver == 0)
			{
				// 無敵タイマーのカウントダウン
				if (invincibleTimer > 0)
				{
					invincibleTimer--;
					if (invincibleTimer == 0)
					{
						StopSoundMem(soundInvincible);
						PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
					}
				}

				// 背景のスクロール処理
				bgX = bgX - 2;
				if (bgX <= -WIDTH) bgX = bgX + WIDTH;

				// ジャンプ処理
				if (CheckHitKey(KEY_INPUT_SPACE) == 1 && playerY == FLOOR_Y)
				{
					playerVY = -20;
				}

				// 重力と移動の計算
				playerVY += 1;
				playerY += playerVY;

				if (playerY > FLOOR_Y)
				{
					playerY = FLOOR_Y;
					playerVY = 0;
				}

				// 障害物の移動
				obstacleX -= obstacleSpeed;
				if (obstacleX < -OBSTACLE_WIDTH)
				{
					obstacleX = WIDTH;
					obstacleY = FLOOR_Y + 20;
					score += 10;
					if (score > hiscore) hiscore = score;
				}

				// 追加アイテムの移動
				itemX -= itemSpeed;
				if (itemX < -ITEM_WIDTH)
				{
					itemX = WIDTH + rand() % 2000 + rand() % 2500;
					itemY = FLOOR_Y - 100;
					isItemExit = 1;
				}

				// スコアアップアイテムの移動
				scoreUpX -= scoreUpSpeed;
				if (scoreUpX < -SCOREUP_WIDTH)
				{
					scoreUpX = WIDTH + rand() % 2000 + rand() % 2500;
					scoreUpY = FLOOR_Y - (60 + rand() % 60); // 毎回少し高さを変えるとおもしろくなります
					isScoreUpExit = 1;

					// 無敵アイテムとの距離を計算し、近すぎたら
					// スコアアップアイテムをさらに右へズラして重なりを防ぐ
					if (abs(scoreUpX - itemX) < 400)
					{
						scoreUpX += 500;
					}
				}

				// 座標と当たり判定用変数の更新
				DrawBikeAndHitBox(playerX, playerY, playerType,
					&rearLeft, &rearRight, &rearTop, &rearBottom,
					&frontLeft, &frontRight, &frontTop, &frontBottom);

				// 各種中心座標の計算
				int oCenter_X = obstacleX + 80; int oCenter_Y = obstacleY + 95; int oCenter_R = 45;
				int oLeft_X = obstacleX + 35;   int oLeft_Y = obstacleY + 130;  int oLeft_R = 25;
				int oRight_X = obstacleX + 120; int oRight_Y = obstacleY + 130; int oRight_R = 25;

				int iCenter_X = itemX + (ITEM_WIDTH / 2);
				int iCenter_Y = itemY + (ITEM_HEIGHT / 2);
				int iCenter_R = 30;

				int rearCenterX = (rearLeft + rearRight) / 2;
				int rearCenterY = (rearTop + rearBottom) / 2;
				int rearRadius = (rearRight - rearLeft) / 2;

				int frontCenterX = (frontLeft + frontRight) / 2;
				int frontCenterY = (frontTop + frontBottom) / 2;
				int frontRadius = (frontRight - frontLeft) / 2;

				// アイテム衝突判定
				if (isItemExit == 1)
				{
					if (CheckCircleHit(rearCenterX, rearCenterY, rearRadius, iCenter_X, iCenter_Y, iCenter_R) ||
						CheckCircleHit(frontCenterX, frontCenterY, frontRadius, iCenter_X, iCenter_Y, iCenter_R))
					{
						isItemExit = 0;
						invincibleTimer = 180;
						score += 30;
						if (score > hiscore) hiscore = score;

						PlaySoundMem(soundItem, DX_PLAYTYPE_BACK, TRUE);
						StopSoundMem(soundBGM);
						PlaySoundMem(soundInvincible, DX_PLAYTYPE_BACK, TRUE);
					}
				}

				// スコアアップアイテム衝突判定
				if (isScoreUpExit == 1)
				{
					// アイテムの中心座標と判定半径の計算
					int suCenter_X = scoreUpX + (SCOREUP_WIDTH / 2);
					int suCenter_Y = scoreUpY + (SCOREUP_HEIGHT / 2);
					int suCenter_R = 25; // 判定の大きさ

					// バイクの後輪または前輪が当たったかチェック
					if (CheckCircleHit(rearCenterX, rearCenterY, rearRadius, suCenter_X, suCenter_Y, suCenter_R) ||
						CheckCircleHit(frontCenterX, frontCenterY, frontRadius, suCenter_X, suCenter_Y, suCenter_R))
					{
						isScoreUpExit = 0;
						score += 100; // スコアを大きく加算（例：100点）
						if (score > hiscore) hiscore = score;

						// 効果音を鳴らす
						PlaySoundMem(soundScoreUP, DX_PLAYTYPE_BACK, TRUE);
					}
				}

				// 障害物衝突判定
				bool isRearHit = CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oCenter_X, oCenter_Y, oCenter_R) ||
					CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oLeft_X, oLeft_Y, oLeft_R) ||
					CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oRight_X, oRight_Y, oRight_R);

				bool isFrontHit = CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oCenter_X, oCenter_Y, oCenter_R) ||
					CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oLeft_X, oLeft_Y, oLeft_R) ||
					CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oRight_X, oRight_Y, oRight_R);

				if (isRearHit || isFrontHit)
				{
					if (invincibleTimer > 0)
					{
						obstacleX = WIDTH;
						obstacleY = FLOOR_Y + 20;
						score += 20;
						if (score > hiscore) hiscore = score;
						PlaySoundMem(soundDestroy, DX_PLAYTYPE_BACK, TRUE);
					}
					else
					{
						isGameOver = 1;
						StopSoundMem(soundBGM);
						StopSoundMem(soundInvincible);
						PlaySoundMem(soundHit, DX_PLAYTYPE_BACK, TRUE);
					}
				}
			}
			else // ゲームオーバー中の入力処理
			{
				// Rキーでリスタート
				if (CheckHitKey(KEY_INPUT_R) == 1)
				{
					playerX = 100;
					playerY = FLOOR_Y;
					playerVY = 0;
					obstacleX = WIDTH;
					obstacleY = FLOOR_Y + 20;
					itemX = WIDTH + rand() % 2000;
					itemY = FLOOR_Y - 100;
					isItemExit = 1;
					scoreUpX = WIDTH + rand() % 2000 + 1000;
					isScoreUpExit = 1;

					invincibleTimer = 0;
					isGameOver = 0;
					score = 0;

					StopSoundMem(soundInvincible);
					PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);

					// リスタート直後の即死を防ぐため、一度座標と判定を強制更新
					DrawBikeAndHitBox(playerX, playerY, playerType, &rearLeft, &rearRight, &rearTop, &rearBottom, &frontLeft, &frontRight, &frontTop, &frontBottom);
				}
				// Tキーでタイトルに戻る
				if (CheckHitKey(KEY_INPUT_T) == 1)
				{
					gameState = 0;
				}
			}
		}

		// 背景の描画
		DrawGraph(bgX, 0, imgBG, FALSE);
		DrawGraph(bgX + WIDTH, 0, imgBG, FALSE);

		if (gameState == 0)
		{
			DrawString(WIDTH / 2 - 70, HEIGHT / 2 - 20, "BIKE GAME", GetColor(255, 255, 255));
			DrawString(WIDTH / 2 - 140, HEIGHT / 2 + 10, "Press Space Key to Start", GetColor(255, 255, 0));
		}
		else if (gameState == 1)
		{
			// 障害物・アイテムの描画
			DrawExtendGraph(obstacleX, obstacleY + 20, obstacleX + OBSTACLE_WIDTH, obstacleY + OBSTACLE_HEIGHT + 20, imgObstacle, TRUE);
			if (isItemExit == 1)
			{
				DrawExtendGraph(itemX, itemY, itemX + ITEM_WIDTH, itemY + ITEM_HEIGHT, imgItem, TRUE);
			}

			// スコアアップアイテムの描画
			if (isScoreUpExit == 1)
			{
				DrawExtendGraph(scoreUpX, scoreUpY, scoreUpX + SCOREUP_WIDTH, scoreUpY + SCOREUP_HEIGHT, imgScoreUP, TRUE);
			}

			// バイクの描画（無敵時の点滅処理含む）
			if (invincibleTimer == 0 || (invincibleTimer / 4) % 2 == 0)
			{
				// ※ここでは実際の描画のみを行う
				DrawGraph(playerX, playerY, imgBike[playerType], TRUE);
			}

			// UIの描画
			DrawFormatString(20, 20, GetColor(255, 255, 255), "SCORE: %d", score);
			DrawFormatString(20, 50, GetColor(255, 255, 0), "HISCORE: %d", hiscore);

			if (invincibleTimer > 0)
			{
				DrawBox(20, 80, 20 + invincibleTimer, 95, GetColor(255, 128, 0), TRUE);
				DrawString(25, 80, "INVINCIBLE!", GetColor(255, 255, 255));
			}

			if (isGameOver == 1)
			{
				DrawString(WIDTH / 2 - 100, HEIGHT / 2 - 20, "GAME OVER", GetColor(255, 0, 0));
				DrawString(WIDTH / 2 - 140, HEIGHT / 2 + 10, "Press R Key to Restart / T Key to Title", GetColor(255, 255, 255));
			}
		}

		// 表画面に反映
		ScreenFlip();
		WaitTimer(16);
	}

	DxLib_End();
	return 0;
}