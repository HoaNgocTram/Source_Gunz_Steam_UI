// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

struct SampleConstants
{
	/** The product id for the running application, found on the dev portal */
	static constexpr char ProductId[] = "a91729130eb949eaa22ef2e3bd7ea5cb";

	/** The sandbox id for the running application, found on the dev portal */
	static constexpr char SandboxId[] = "61afb5ef566148b0bfdda3bba8b80407";

	/** The deployment id for the running application, found on the dev portal */
	static constexpr char DeploymentId[] = "df8f76f8a0714673bf45cda2cb81c7d9";

	/** Client id of the service permissions entry, found on the dev portal */
	static constexpr char ClientCredentialsId[] = "xyza7891guWoBMPDSNQp8Ceio1GLqTX3";

	/** Client secret for accessing the set of permissions, found on the dev portal */
	static constexpr char ClientCredentialsSecret[] = "j0ktCFEErPzohdU/dskYNj+vZncXhgH4bPcWV8VaiGI";

	/** Game name */
	static constexpr char GameName[] = "Gunz VN";

	/** Encryption key. Not used by this sample. */
	static constexpr char EncryptionKey[] = "1111111111111111111111111111111111111111111111111111111111111111";

	/** The Minimum window Width for this sample. */
	static constexpr int32_t MinimumWindowWidth = 1024;

	/** The Minimum window Height for this sample. */
	static constexpr int32_t MinimumWindowHeight = 800;

	/** The Default window Width for this sample. */
	static constexpr int32_t DefaultWindowWidth = 1024;

	/** The Default window Height for this sample. */
	static constexpr int32_t DefaultWindowHeight = 800;
};